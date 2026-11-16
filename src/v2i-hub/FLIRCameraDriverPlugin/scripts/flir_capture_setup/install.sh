#!/usr/bin/env bash
# Install ../flir_capture.sh as a systemd service on this machine (weekdays 07:00-18:00, 20 GB capture ring).
#
# Asks for the camera, switch and control addresses, checks that they answer, shows the resulting command,
# and installs after confirmation. The answers are saved to /etc/flir-capture/site.conf and offered as the
# defaults next time, so rerun this script to change them.
#
#   ./install.sh             ask, then install
#   ./install.sh --dry-run   ask and print the service file; install nothing
#   ./install.sh --yes       reinstall with the saved answers, without asking
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPTS_DIR="$(cd "$HERE/.." && pwd)"
CONF="${FLIR_CAPTURE_CONF:-/etc/flir-capture/site.conf}"
DRY_RUN=0 YES=0
for a in "$@"; do
    case $a in
        --dry-run) DRY_RUN=1 ;;
        --yes|-y) YES=1 ;;
        -h|--help) sed -n '2,10p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "unknown option: $a (see: $0 --help)" >&2; exit 1 ;;
    esac
done

die() { echo "error: $*" >&2; exit 1; }
[ -t 0 ] || [ "$YES" = 1 ] || die "run this in a terminal (it asks for the site's addresses), or use --yes to reuse the saved answers."

# Saved answers from the last install, if any
RUN_USER="" CAMERA="" IFACE="" SWITCH="" SWITCH_PORT="" CONTROLS="" SELF_CHECK="" EXTRA_ADDR="" NM_CON="" FIRMWARE="" NOTE=""
SAVED=0
if [ -r "$CONF" ]; then
    # shellcheck disable=SC1090
    . "$CONF"
    SAVED=1
    echo "Using the answers saved in $CONF as defaults."
elif [ "$YES" = 1 ]; then
    die "--yes needs saved answers in $CONF; run without --yes first."
fi

# ---- validators (set ERR and return 1 when the value is not valid) -------------------------------------
ERR=""
is_ipv4() {
    [[ $1 =~ ^([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})$ ]] || return 1
    local i; for i in 1 2 3 4; do [ "${BASH_REMATCH[$i]}" -le 255 ] || return 1; done
}
is_port() { [[ $1 =~ ^[0-9]{1,5}$ ]] && [ "$1" -ge 1 ] && [ "$1" -le 65535 ]; }
is_hostport() { [[ $1 =~ ^[A-Za-z0-9.-]+:([0-9]+)$ ]] && is_port "${BASH_REMATCH[1]}"; }
v_user() { id -u "$1" >/dev/null 2>&1 || { ERR="no such user"; return 1; }; }
v_ip() { is_ipv4 "$1" || { ERR="enter an IPv4 address such as 192.168.55.183"; return 1; }; }
v_switch() { [ "$1" = none ] || is_ipv4 "$1" || { ERR="enter an IPv4 address, or none"; return 1; }; }
v_port_opt() { [ -z "$1" ] || is_port "$1" || { ERR="enter a port number 1-65535, or - for none"; return 1; }; }
v_iface() { [ -n "$1" ] && ip link show dev "$1" >/dev/null 2>&1 || { ERR="no such interface (see: ip -br link)"; return 1; }; }
v_hostport_opt() { [ -z "$1" ] || is_hostport "$1" || { ERR="enter HOST:PORT, or - to skip"; return 1; }; }
v_hostports() {
    local h; for h in $1; do is_hostport "$h" || { ERR="'$h' is not HOST:PORT; separate several with spaces"; return 1; }; done
}
v_cidr_opt() {
    [ -z "$1" ] && return 0
    local addr="${1%/*}" prefix="${1##*/}"
    [[ $1 == */* && $prefix =~ ^[0-9]{1,2}$ ]] && [ "$prefix" -le 32 ] && is_ipv4 "$addr" ||
        { ERR="enter ADDRESS/PREFIX such as 192.168.0.52/24, or - for none"; return 1; }
}
v_nmcon_opt() {
    [ -z "$1" ] && return 0
    nmcli -g NAME connection show 2>/dev/null | grep -Fxq -- "$1" || { ERR="no such NetworkManager connection (see: nmcli connection show)"; return 1; }
}
v_any() { :; }

# Default for an optional value: the saved answer (even if it was left empty), else a suggestion.
saved_or() { if [ "$SAVED" = 1 ]; then printf '%s' "$1"; else printf '%s' "$2"; fi; }

# ask VAR VALIDATOR "question" DEFAULT [optional]
# Enter keeps the default; "-" clears an optional value.
ask() {
    local var=$1 check=$2 question=$3 def=$4 optional=${5:-} ans
    while true; do
        if [ "$YES" = 1 ]; then
            ans=$def
        else
            read -r -p "$question${def:+ [$def]}: " ans || die "no answer"
            ans=${ans:-$def}
            [ "$ans" = - ] && [ -n "$optional" ] && ans=""
        fi
        if [ -z "$ans" ] && [ -z "$optional" ]; then
            [ "$YES" = 1 ] && die "no saved value for: $question"
            echo "  a value is needed"; continue
        fi
        if "$check" "$ans"; then printf -v "$var" '%s' "$ans"; return 0; fi
        [ "$YES" = 1 ] && die "saved value '$ans' is not valid for: $question ($ERR)"
        echo "  $ERR"
    done
}

# ---- questions ------------------------------------------------------------------------------------------
if [ "$YES" = 0 ]; then
    cat <<'EOF'
FLIR capture service setup. Press Enter to keep the value in [brackets]; type - to leave an optional value empty.
See README.md next to this script for a diagram of what is checked.

EOF
fi

ask RUN_USER v_user "User the service runs as (needs to read $SCRIPTS_DIR)" "${RUN_USER:-${SUDO_USER:-$USER}}"
ask CAMERA v_ip "FLIR camera IP address" "$CAMERA"

route_dev() { ip route get "$1" 2>/dev/null | grep -oP ' dev \K\S+' | head -1; }
route_src() { ip route get "$1" 2>/dev/null | grep -oP ' src \K\S+' | head -1; }
ask IFACE v_iface "Network interface that reaches the camera (captured on)" "${IFACE:-$(route_dev "$CAMERA" || true)}"

ask SWITCH v_switch "PoE switch IP, pinged next to the camera (or another device on the same switch; none to skip)" "${SWITCH:-}"
if [ "$SWITCH" = none ]; then
    SWITCH_PORT="" EXTRA_ADDR="" NM_CON=""
else
    ask SWITCH_PORT v_port_opt "Open TCP port on the switch, e.g. 80 for its web page (- for none)" "$SWITCH_PORT" optional
    if [ -z "$EXTRA_ADDR" ] && ip route get "$SWITCH" 2>/dev/null | grep -q ' via '; then
        echo "  $SWITCH is not on a subnet $IFACE is attached to. If the switch only has its factory address,"
        echo "  give this host an extra address on the switch's subnet, e.g. 192.168.0.52/24 for a switch at 192.168.0.239."
    fi
    ask EXTRA_ADDR v_cidr_opt "Extra address to add to $IFACE to reach the switch (- for none)" "$EXTRA_ADDR" optional
    if [ -n "$EXTRA_ADDR" ] && command -v nmcli >/dev/null 2>&1; then
        ask NM_CON v_nmcon_opt "NetworkManager connection to save that address in (- to not save it; the service adds it at each start anyway)" \
            "$(saved_or "$NM_CON" "$(nmcli -g GENERAL.CONNECTION device show "$IFACE" 2>/dev/null || true)")" optional
    else
        NM_CON=""
    fi
fi

default_gw=$(ip route show default dev "$IFACE" 2>/dev/null | awk '{print $3; exit}' || true)
ask CONTROLS v_hostports "Control hosts, HOST:PORT with an open TCP port, e.g. the gateway or RSU (space-separated, - for none)" \
    "$(saved_or "$CONTROLS" "${default_gw:+$default_gw:80}")" optional
src=$(route_src "$CAMERA" || true)
ask SELF_CHECK v_hostport_opt "This host's own address for the self check, HOST:PORT with an open port such as SSH (- to skip)" \
    "$(saved_or "$SELF_CHECK" "${src:+$src:22}")" optional
ask FIRMWARE v_any "Camera firmware version, recorded with each run (- for none)" "$FIRMWARE" optional
ask NOTE v_any "Note recorded with each run: switch, port, cable, time source (- for none)" "$NOTE" optional

# ---- reachability (warnings only) -----------------------------------------------------------------------
echo
echo "Checking the addresses from this host:"
check_ping() { if ping -c 1 -W 1 "$1" >/dev/null 2>&1; then echo "  ok         ping $1 ($2)"; else echo "  NO ANSWER  ping $1 ($2)"; fi; }
check_tcp() {
    if timeout 2 bash -c "exec 3<>/dev/tcp/$1/$2" 2>/dev/null; then echo "  ok         tcp  $1:$2 ($3)"; else echo "  NO ANSWER  tcp  $1:$2 ($3)"; fi
}
check_ping "$CAMERA" camera
check_tcp "$CAMERA" 80 "camera WebSocket"
if [ "$SWITCH" != none ]; then
    if [ -n "$EXTRA_ADDR" ] && ! ip -4 addr show dev "$IFACE" | grep -qF "inet ${EXTRA_ADDR%/*}/"; then
        echo "  (skipped)  ping $SWITCH (switch): $IFACE does not have ${EXTRA_ADDR} yet; the service adds it"
    else
        check_ping "$SWITCH" switch
        [ -z "$SWITCH_PORT" ] || check_tcp "$SWITCH" "$SWITCH_PORT" "switch"
    fi
fi
for h in $CONTROLS; do check_tcp "${h%:*}" "${h##*:}" control; done
[ -z "$SELF_CHECK" ] || check_tcp "${SELF_CHECK%:*}" "${SELF_CHECK##*:}" "self check"

# ---- build the service file -----------------------------------------------------------------------------
# Free text goes inside "..." in the unit file: drop quotes and backslashes, escape systemd's % and $.
unit_text() { local s=${1//[\"\\]/}; s=${s//%/%%}; s=${s//\$/\$\$}; printf '"%s"' "$s"; }

args="--camera $CAMERA --switch $SWITCH"
[ -z "$SWITCH_PORT" ] || args+=" --switch-port $SWITCH_PORT"
for h in $CONTROLS $SELF_CHECK; do args+=" --control $h"; done
args+=" --iface $IFACE --ring-mb 100 --ring-files 200"
[ -z "$FIRMWARE" ] || args+=" --firmware $(unit_text "$FIRMWARE")"
note="${NOTE:+$NOTE; }started by flir-capture.service"
args+=" --note $(unit_text "$note")"

if [ -n "$EXTRA_ADDR" ]; then
    extra="# Make sure this host has its address on the switch's subnet
ExecStartPre=+/bin/sh -c 'ip -4 addr show dev $IFACE | grep -qF \"inet ${EXTRA_ADDR%/*}/\" || ip addr add $EXTRA_ADDR dev $IFACE'"
else
    extra="# No extra address needed to reach the switch"
fi

group=$(id -gn "$RUN_USER")
unit="# Generated by flir_capture_setup/install.sh from systemd/flir-capture.service.in. Rerun install.sh to change it.
$(tail -n +3 "$HERE/systemd/flir-capture.service.in")"
unit=${unit//@USER@/"$RUN_USER"}
unit=${unit//@GROUP@/"$group"}
unit=${unit//@SCRIPTS_DIR@/"$SCRIPTS_DIR"}
unit=${unit//@CAMERA@/"$CAMERA"}
unit=${unit//@EXTRA_ADDRESS@/"$extra"}
unit=${unit//@START_ARGS@/"$args"}

echo
echo "The service will run, as $RUN_USER on weekdays 07:00-18:00:"
echo "  $SCRIPTS_DIR/flir_capture.sh start $args"
[ -z "$EXTRA_ADDR" ] || echo "and first add $EXTRA_ADDR to $IFACE${NM_CON:+ (also saved in NetworkManager connection $NM_CON)}."

if [ "$DRY_RUN" = 1 ]; then
    echo; echo "--- flir-capture.service (dry run, nothing installed)"; printf '%s\n' "$unit"
    exit 0
fi
if [ "$YES" = 0 ]; then
    read -r -p "Install and (re)start the service with these settings? [Y/n] " ok
    case ${ok:-y} in [Yy]*) ;; *) echo "Nothing installed."; exit 0 ;; esac
fi

# ---- install --------------------------------------------------------------------------------------------
sudo mkdir -p "$(dirname "$CONF")"
{
    echo "# Answers from flir_capture_setup/install.sh, offered as defaults when it is run again."
    for v in RUN_USER CAMERA IFACE SWITCH SWITCH_PORT CONTROLS SELF_CHECK EXTRA_ADDR NM_CON FIRMWARE NOTE; do
        printf '%s=%q\n' "$v" "${!v}"
    done
} | sudo tee "$CONF" >/dev/null

# tshark, with dumpcap allowed for members of the wireshark group
echo "wireshark-common wireshark-common/install-setuid boolean true" | sudo debconf-set-selections
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y tshark
sudo DEBIAN_FRONTEND=noninteractive dpkg-reconfigure -f noninteractive wireshark-common
sudo usermod -aG wireshark "$RUN_USER"

if [ -n "$NM_CON" ]; then
    if nmcli -g ipv4.addresses connection show "$NM_CON" | tr ',' '\n' | sed 's/^ *//' | grep -Fxq "$EXTRA_ADDR"; then
        echo "$EXTRA_ADDR is already saved in NetworkManager connection $NM_CON."
    else
        sudo nmcli connection modify "$NM_CON" +ipv4.addresses "$EXTRA_ADDR" ||
            echo "warning: could not save $EXTRA_ADDR in $NM_CON; the service still adds it at each start."
    fi
fi

sudo install -m 755 "$HERE/flir-capture-in-hours" /usr/local/bin/flir-capture-in-hours
sudo install -m 644 "$HERE/systemd/flir-capture-start.timer" "$HERE/systemd/flir-capture-stop.timer" \
    "$HERE/systemd/flir-capture-stop.service" /etc/systemd/system/
printf '%s\n' "$unit" | sudo tee /etc/systemd/system/flir-capture.service >/dev/null
sudo systemctl daemon-reload
sudo systemctl enable flir-capture.service flir-capture-start.timer flir-capture-stop.timer
sudo systemctl start flir-capture-start.timer flir-capture-stop.timer
# Restart so new settings take effect now; outside Mon-Fri 07:00-18:00 the service skips itself.
sudo systemctl restart flir-capture.service
echo
systemctl list-timers 'flir-capture-*' --no-pager
echo
echo "Installed. Check with: systemctl status flir-capture   and   $SCRIPTS_DIR/flir_capture.sh status"
