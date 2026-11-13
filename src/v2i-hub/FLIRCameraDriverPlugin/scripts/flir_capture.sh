#!/usr/bin/env bash
# Collect evidence for FLIR camera link stalls (detections stop for ~3.5 s, then arrive in a burst).
#
# Runs, unattended and all into one run directory:
#   - flir_stream_probe.py live: second WebSocket client on the camera, stall detection, TCP connect checks
#   - ICMP ping (timestamped) to the camera and the PoE switch
#   - TCP connect checks to the camera's RTSP port
#   - a rotating packet capture (dumpcap) of all traffic to/from the camera and the switch
#   - a log of this host's own link/address changes
# and cuts out a small, self-contained bundle per stall to send to the camera vendor.
#
#   ./flir_capture.sh start --switch 192.168.55.2 --firmware V2.05 --note "switch A, port 3, cable 1"
#   ./flir_capture.sh status
#   ./flir_capture.sh list                         # stalls found so far, with UTC times
#   ./flir_capture.sh extract 2026-11-13T15:15:02.120Z
#   ./flir_capture.sh stop
#
# Run "./flir_capture.sh help" for all options.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROBE="${PROBE:-$HERE/flir_stream_probe.py}"
RUNS_DIR="${RUNS_DIR:-$HERE/flir_capture_runs}"

usage() {
    cat <<'EOF'
Usage: flir_capture.sh <command> [options]

Commands
  start      Start collecting into a new run directory (flir_capture_runs/<date>_<time>/). Keeps running after logout.
  status     Show what is running, capture size, and stalls / ping losses found so far.
  list       List the stalls, silences and late bursts found so far (UTC times, for "extract").
  extract    Cut the capture and logs around one event into <run>/events/<time>/ and a .tar.gz.
  stop       Stop everything cleanly (the probe writes its summary on stop).

start options
  --camera IP          Camera address (default 192.168.55.183).
  --switch IP          PoE switch address, pinged next to the camera. If the switch has no IP address,
                       use another device plugged into the same switch. "none" to skip.
  --switch-port PORT   An open TCP port on the switch (e.g. 80 for its web page), also checked with TCP
                       connects by the probe. Leave out if the switch has no open TCP port.
  --control HOST:PORT  Extra host for the probe's TCP comparison checks, e.g. the gateway (repeatable).
                       Must be another machine with an open TCP port.
  --ping HOST          Extra host to ping and capture (repeatable).
  --rtsp-port PORT     Camera RTSP port to check with TCP connects (default 554, 0 to skip).
  --iface IF           Capture interface (default: the one routing to the camera).
  --ring-mb N          Size of each capture file in MB (default 100).
  --ring-files N       Number of capture files kept; the oldest is overwritten (default 100).
  --firmware TEXT      Camera firmware version, recorded in notes.txt.
  --note TEXT          Free text recorded in notes.txt: switch, port, cable, what changed (repeatable).
  --no-capture         Do not run the packet capture (only if capturing is not possible).

extract <UTC time> [options]
  Time as printed by "list" (e.g. 2026-11-13T15:15:02.120Z). A time without a zone is taken as UTC.
  --before S           Seconds before the time to include (default 30).
  --after S            Seconds after the time to include (default 30).

status / list / extract / stop take --run DIR to use a run other than the latest.
EOF
}

die() { echo "error: $*" >&2; exit 1; }
warn() { echo "warning: $*" >&2; }
need() { command -v "$1" >/dev/null 2>&1 || die "$1 not found. $2"; }

latest_run() {
    [ -L "$RUNS_DIR/latest" ] || die "no run found in $RUNS_DIR. Start one with: $0 start"
    readlink -f "$RUNS_DIR/latest"
}

# Prints "name pid" for each process of a run that is still running
alive() {
    local run=$1 name pid
    [ -f "$run/pids" ] || return 0
    while read -r name pid; do
        if kill -0 "$pid" 2>/dev/null && [ -r "/proc/$pid/cmdline" ]; then
            echo "$name $pid"
        fi
    done < "$run/pids"
}

launch() {  # launch <name> <logfile> <command...>
    local name=$1 log=$2; shift 2
    setsid nohup "$@" > "$log" 2>&1 < /dev/null &
    echo "$name $!" >> "$RUN/pids"
}

# TCP connect checks of one HOST:PORT every 0.5 s. CSV: epoch_s,utc,target,duration_ms,ok,error
TCPCHECK='
import socket, sys, time, errno, datetime
host, port, out = sys.argv[1], int(sys.argv[2]), sys.argv[3]
with open(out, "a", buffering=1) as f:
    if f.tell() == 0:
        f.write("epoch_s,utc,target,duration_ms,ok,error\n")
    while True:
        t = time.time(); m = time.monotonic(); ok, err = 1, ""
        try:
            socket.create_connection((host, port), timeout=3).close()
        except OSError as e:
            ok, err = 0, (errno.errorcode.get(e.errno) if e.errno else None) or type(e).__name__
        d = int((time.monotonic() - m) * 1000)
        u = datetime.datetime.fromtimestamp(t, datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%S.%f")[:-3] + "Z"
        f.write(f"{t:.3f},{u},{host}:{port},{d},{ok},{err}\n")
        time.sleep(max(0.0, 0.5 - (time.monotonic() - m)))
'

cmd_start() {
    local camera=192.168.55.183 switch="" switch_port="" rtsp_port=554 iface="" ring_mb=100 ring_files=100
    local firmware="" capture=1
    local -a controls=() pings=() notes=()
    while [ $# -gt 0 ]; do
        case $1 in
            --camera) camera=$2; shift 2 ;;
            --switch) switch=$2; shift 2 ;;
            --switch-port) switch_port=$2; shift 2 ;;
            --control) controls+=("$2"); shift 2 ;;
            --ping) pings+=("$2"); shift 2 ;;
            --rtsp-port) rtsp_port=$2; shift 2 ;;
            --iface) iface=$2; shift 2 ;;
            --ring-mb) ring_mb=$2; shift 2 ;;
            --ring-files) ring_files=$2; shift 2 ;;
            --firmware) firmware=$2; shift 2 ;;
            --note) notes+=("$2"); shift 2 ;;
            --no-capture) capture=0; shift ;;
            *) die "unknown option for start: $1 (see: $0 help)" ;;
        esac
    done
    [ -n "$switch" ] || die "--switch is required: the PoE switch's IP, or another device on the same switch. Use --switch none to skip."
    [ "$switch" = none ] && switch=""
    [ -z "$switch_port" ] || [ -n "$switch" ] || die "--switch-port needs --switch"

    need python3 "Install Python 3."
    need ping "Install iputils-ping."
    [ -f "$PROBE" ] || die "probe not found at $PROBE (set PROBE=/path/to/flir_stream_probe.py)"

    if [ -L "$RUNS_DIR/latest" ] && [ -n "$(alive "$(readlink -f "$RUNS_DIR/latest")")" ]; then
        die "a run is still going in $(readlink -f "$RUNS_DIR/latest"). Stop it first: $0 stop"
    fi

    if [ -z "$iface" ]; then
        iface=$(ip route get "$camera" 2>/dev/null | grep -oP ' dev \K\S+' | head -1) || true
        [ -n "$iface" ] || die "no route to $camera. Pass --iface."
    fi

    local -a hosts=("$camera")
    [ -n "$switch" ] && hosts+=("$switch")
    hosts+=("${pings[@]}")
    local filter="" h
    for h in "${hosts[@]}"; do filter+="${filter:+ or }host $h"; done

    if [ "$capture" = 1 ]; then
        need dumpcap "Install wireshark-common (sudo apt install tshark)."
        if ! dumpcap -D 2>/dev/null | grep -qE "^[0-9]+\. $iface( |$)"; then
            die "this user cannot capture on $iface. Run once:
    sudo usermod -aG wireshark \$USER
then log out and back in (or run: newgrp wireshark), and check that 'dumpcap -D' lists $iface.
If dumpcap still cannot capture: sudo dpkg-reconfigure wireshark-common (answer Yes)."
        fi
        local free_mb need_mb
        free_mb=$(df -Pm "$HERE" | awk 'NR==2 {print $4}')
        need_mb=$((ring_mb * ring_files))
        [ "$free_mb" -gt $((need_mb + need_mb / 5)) ] ||
            die "the capture can use up to ${need_mb} MB but only ${free_mb} MB is free. Lower --ring-files or --ring-mb."
    fi

    # Warnings that do not stop the run
    ping -c 1 -W 1 "$camera" >/dev/null 2>&1 || warn "$camera does not answer ping right now."
    if [ "$(timedatectl show -p NTPSynchronized --value 2>/dev/null || true)" != yes ]; then
        warn "this host's clock is not NTP-synchronized; event times will not line up with the camera's logs."
    fi
    if [ "$(systemd-detect-virt 2>/dev/null || true)" != none ]; then
        warn "this is a virtual machine. If its network adapter is NAT, the hypervisor relays TCP and the capture will
         not show the camera's real retransmissions; use a bridged adapter or a physical machine."
    fi

    mkdir -p "$RUNS_DIR"
    RUN="$RUNS_DIR/$(date +%Y%m%d_%H%M%S)"
    mkdir -p "$RUN"
    ln -sfn "$(basename "$RUN")" "$RUNS_DIR/latest"

    {
        echo "started_utc: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
        echo "started_epoch: $(date +%s)"
        echo "host: $(hostname)  iface: $iface  virt: $(systemd-detect-virt 2>/dev/null || echo unknown)"
        echo "camera: $camera  firmware: ${firmware:-not recorded}"
        echo "switch: ${switch:-none}${switch_port:+  tcp port $switch_port}"
        echo "extra ping: ${pings[*]:-none}  extra controls: ${controls[*]:-none}  rtsp port: $rtsp_port"
        echo "capture: $([ "$capture" = 1 ] && echo "$iface, filter '$filter', ${ring_files} x ${ring_mb} MB ring" || echo off)"
        echo "ntp synchronized: $(timedatectl show -p NTPSynchronized --value 2>/dev/null || echo unknown)"
        for h in "${notes[@]}"; do echo "note: $h"; done
        echo
        echo "--- ip addr"; ip -br addr
        echo "--- route to camera"; ip route get "$camera" 2>&1 || true
    } > "$RUN/notes.txt"

    local -a probe_args=(live --host "$camera" --out "$RUN")
    [ -n "$switch_port" ] && probe_args+=(--control "$switch:$switch_port")
    for h in "${controls[@]}"; do probe_args+=(--control "$h"); done
    launch probe "$RUN/console.log" python3 -u "$PROBE" "${probe_args[@]}"

    launch ping_camera "$RUN/ping_camera.log" ping -D -O -i 0.2 "$camera"
    [ -n "$switch" ] && launch ping_switch "$RUN/ping_switch.log" ping -D -O -i 0.2 "$switch"
    for h in "${pings[@]}"; do launch "ping_$h" "$RUN/ping_$h.log" ping -D -O -i 0.2 "$h"; done

    [ "$rtsp_port" != 0 ] && launch rtsp_check "$RUN/rtsp_check.log" \
        python3 -I -c "$TCPCHECK" "$camera" "$rtsp_port" "$RUN/rtsp_check.csv"

    launch ip_monitor "$RUN/ip_monitor.log" ip -ts monitor link address

    if [ "$capture" = 1 ]; then
        launch dumpcap "$RUN/dumpcap.log" dumpcap -q -i "$iface" -f "$filter" \
            -b "filesize:$((ring_mb * 1000))" -b "files:$ring_files" -w "$RUN/capture.pcapng"
    fi

    sleep 3
    local started failed=""
    started=$(alive "$RUN" | awk '{print $1}' | tr '\n' ' ')
    while read -r name _; do
        grep -qw "$name" <<<"$started" || failed+="$name "
    done < "$RUN/pids"
    echo "Run directory: $RUN"
    echo "Running: $started"
    if [ -n "$failed" ]; then
        warn "these stopped right away: $failed(see their .log files in the run directory)"
        exit 1
    fi
    cat <<EOF

Leave it running. Check with "$0 status"; "$0 list" shows the stalls found so far.
After a stall: "$0 extract <time from list>", then download the camera's logs from its web interface.
EOF
}

cmd_status() {
    local run=$1 started now
    [ -d "$run" ] || die "no run directory $run"
    started=$(awk '/^started_epoch:/ {print $2}' "$run/notes.txt")
    now=$(date +%s)
    echo "Run: $run"
    echo "Started: $(awk '/^started_utc:/ {print $2}' "$run/notes.txt") ($(( (now - started) / 3600 ))h $(( (now - started) % 3600 / 60 ))m ago)"
    echo
    local name pid running
    running=$(alive "$run")
    if [ -f "$run/pids" ]; then
        while read -r name pid; do
            if grep -q "^$name " <<<"$running"; then echo "  running  $name"; else echo "  stopped  $name"; fi
        done < "$run/pids"
    fi
    echo
    if ls "$run"/capture_*.pcapng >/dev/null 2>&1; then
        echo "Capture: $(ls "$run"/capture_*.pcapng | wc -l) file(s), $(du -ch "$run"/capture_*.pcapng | tail -1 | cut -f1)," \
             "oldest from $(ls "$run"/capture_*.pcapng | head -1 | grep -oP '_\K\d{14}(?=\.pcapng)' | sed -E 's/(....)(..)(..)(..)(..)(..)/\1-\2-\3 \4:\5:\6/') local time"
    fi
    local f
    for f in "$run"/ping_*.log; do
        [ -f "$f" ] || continue
        echo "$(basename "$f" .log): $(grep -c 'bytes from' "$f" || true) replies, $(grep -c 'no answer yet' "$f" || true) unanswered"
    done
    if [ -f "$run/rtsp_check.csv" ]; then
        echo "rtsp_check: $(awk -F, 'NR>1 && $5==1' "$run/rtsp_check.csv" | wc -l) ok, $(awk -F, 'NR>1 && $5==0' "$run/rtsp_check.csv" | wc -l) failed"
    fi
    echo "ip_monitor: $(cat "$run/ip_monitor.log" 2>/dev/null | grep -c . || true) link/address change(s) on this host"
    echo
    echo "Found by the probe: $(grep -c 'STALL details' "$run/console.log" 2>/dev/null || true) stall(s)," \
         "$(grep -c 'SILENCE details' "$run/console.log" 2>/dev/null || true) silence(s)," \
         "$(grep -c 'LATE BURST' "$run/console.log" 2>/dev/null || true) late burst(s). Times: $0 list"
}

cmd_list() {
    local run=$1
    grep -E 'STALL details|SILENCE details|LATE BURST' "$run/console.log" 2>/dev/null ||
        echo "No stalls, silences or late bursts found yet in $run."
}

cmd_stop() {
    local run=$1 name pid running
    running=$(alive "$run")
    [ -n "$running" ] || { echo "Nothing running in $run."; return; }
    while read -r name pid; do kill -INT "$pid" 2>/dev/null || true; done <<<"$running"
    local i
    for i in $(seq 1 20); do
        [ -z "$(alive "$run")" ] && break
        sleep 0.5
    done
    running=$(alive "$run")
    if [ -n "$running" ]; then
        while read -r name pid; do kill -TERM "$pid" 2>/dev/null || true; done <<<"$running"
        sleep 1
    fi
    echo "Stopped. Run directory: $run"
    [ -z "$(alive "$run")" ] || warn "still running: $(alive "$run" | tr '\n' ' ')"
}

cmd_extract() {
    local run=$1; shift
    local when="" before=30 after=30
    while [ $# -gt 0 ]; do
        case $1 in
            --before) before=$2; shift 2 ;;
            --after) after=$2; shift 2 ;;
            -*) die "unknown option for extract: $1" ;;
            *) when=$1; shift ;;
        esac
    done
    [ -n "$when" ] || die "give the event time, e.g.: $0 extract 2026-11-13T15:15:02.120Z (see: $0 list)"
    need tshark "Install tshark."
    need capinfos "Install wireshark-common."
    need mergecap "Install wireshark-common."

    local t lo hi stamp out camera
    t=$(TZ=UTC date -d "$when" +%s.%N 2>/dev/null) || die "cannot read the time '$when'"
    lo=$(awk -v t="$t" -v b="$before" 'BEGIN {printf "%.3f", t - b}')
    hi=$(awk -v t="$t" -v a="$after" 'BEGIN {printf "%.3f", t + a}')
    stamp=$(date -u -d "@${t%.*}" +%Y%m%dT%H%M%SZ)
    out="$run/events/$stamp"
    mkdir -p "$out"
    camera=$(awk '/^camera:/ {print $2}' "$run/notes.txt")

    # Capture files whose packets overlap the window
    local -a sel=()
    local f s e
    for f in "$run"/capture_*.pcapng; do
        [ -f "$f" ] || continue
        read -r s e < <(capinfos -T -r -a -e -S "$f" 2>/dev/null | awk -F'\t' '{print $2, $3}') || continue
        [[ "$s" =~ ^[0-9.]+$ && "$e" =~ ^[0-9.]+$ ]] || continue
        if awk -v s="$s" -v e="$e" -v lo="$lo" -v hi="$hi" 'BEGIN {exit !(s <= hi && e >= lo)}'; then sel+=("$f"); fi
    done

    local summary="$out/summary.txt"
    {
        echo "Event: $when (UTC), window $(date -u -d "@${lo%.*}" +%H:%M:%S) to $(date -u -d "@${hi%.*}" +%H:%M:%S) UTC"
        echo "Camera: $camera"
        sed -n '1,9p' "$run/notes.txt"
        echo
    } > "$summary"

    if [ ${#sel[@]} -gt 0 ]; then
        local tmp
        tmp=$(mktemp --suffix=.pcapng)
        mergecap -w "$tmp" "${sel[@]}"
        tshark -r "$tmp" -Y "frame.time_epoch >= $lo && frame.time_epoch <= $hi" -w "$out/capture.pcapng" 2>/dev/null
        rm -f "$tmp"
        local cap="$out/capture.pcapng"
        count() { tshark -r "$cap" -Y "$1" 2>/dev/null | wc -l; }
        {
            echo "=== Packet capture ($(count 'frame') packets in the window)"
            echo "From the camera: $(count "ip.src==$camera"), to the camera: $(count "ip.dst==$camera")"
            echo
            echo "Longest gaps between packets from the camera (seconds, gap ended at UTC):"
            tshark -r "$cap" -Y "ip.src==$camera" -T fields -e frame.time_epoch 2>/dev/null |
                awk 'NR > 1 {printf "%.3f %s\n", $1 - p, $1} {p = $1}' | sort -rn | head -5 |
                while read -r gap end; do echo "  $gap s  $(date -u -d "@$end" +%H:%M:%S.%3N)"; done
            echo
            echo "TCP: retransmissions $(count 'tcp.analysis.retransmission || tcp.analysis.fast_retransmission')," \
                 "duplicate ACKs $(count 'tcp.analysis.duplicate_ack'), zero window $(count 'tcp.analysis.zero_window')," \
                 "resets $(count 'tcp.flags.reset==1')"
            echo "TCP connects to the camera: SYN sent $(count "ip.dst==$camera && tcp.flags.syn==1 && tcp.flags.ack==0")," \
                 "SYN-ACK received $(count "ip.src==$camera && tcp.flags.syn==1 && tcp.flags.ack==1")"
            local h
            for h in $(tshark -r "$cap" -Y "icmp.type==8" -T fields -e ip.dst 2>/dev/null | sort -u); do
                echo "Ping $h: requests $(count "icmp.type==8 && ip.dst==$h"), replies $(count "icmp.type==0 && ip.src==$h")"
            done
            echo
            echo "TCP problems flagged by Wireshark (first 40, UTC):"
            tshark -r "$cap" -t ud -Y 'tcp.analysis.flags && !tcp.analysis.window_update' \
                -T fields -E separator=' ' -e _ws.col.Time -e ip.src -e ip.dst -e _ws.col.Info 2>/dev/null | head -40 | sed 's/^/  /'
            echo
        } >> "$summary"
    else
        echo "=== No packet capture covers this time (capture off, not started yet, or already overwritten)." >> "$summary"
        echo >> "$summary"
    fi

    # Logs, cut to the window where they have per-line times
    for f in "$run"/ping_*.log; do
        [ -f "$f" ] || continue
        awk -v lo="$lo" -v hi="$hi" 'match($0, /^\[[0-9.]+\]/) {t = substr($0, 2, RLENGTH - 2); if (t >= lo && t <= hi) print}' \
            "$f" > "$out/$(basename "$f")"
        echo "$(basename "$f" .log): $(grep -c 'bytes from' "$out/$(basename "$f")" || true) replies," \
             "$(grep -c 'no answer yet' "$out/$(basename "$f")" || true) unanswered in the window" >> "$summary"
    done
    if [ -f "$run/rtsp_check.csv" ]; then
        awk -F, -v lo="$lo" -v hi="$hi" 'NR == 1 || ($1 >= lo && $1 <= hi)' "$run/rtsp_check.csv" > "$out/rtsp_check.csv"
        echo "rtsp_check: $(awk -F, 'NR>1 && $5==1' "$out/rtsp_check.csv" | wc -l) ok," \
             "$(awk -F, 'NR>1 && $5==0' "$out/rtsp_check.csv" | wc -l) failed in the window" >> "$summary"
    fi
    for f in frames.csv chunks.csv probes.csv; do
        [ -f "$run/$f" ] && awk -F, -v lo="$lo" -v hi="$hi" 'NR == 1 || ($2 >= lo * 1000 && $2 <= hi * 1000)' \
            "$run/$f" > "$out/$f"
    done
    for f in console.log events.jsonl notes.txt ip_monitor.log; do
        [ -f "$run/$f" ] && cp "$run/$f" "$out/"
    done

    tar -C "$run/events" -czf "$run/events/flir_event_$stamp.tar.gz" "$stamp"
    cat "$summary"
    echo
    echo "Bundle: $run/events/flir_event_$stamp.tar.gz"
    echo "Add the camera's own logs (download them from its web interface now, before they rotate) and send both."
}

command=${1:-help}; [ $# -gt 0 ] && shift
run_arg=""
args=()
while [ $# -gt 0 ]; do
    if [ "$1" = --run ]; then run_arg=$(readlink -f "$2"); shift 2; else args+=("$1"); shift; fi
done
set -- "${args[@]+"${args[@]}"}"

case $command in
    start) [ -z "$run_arg" ] || die "start creates a new run; --run is not used"; cmd_start "$@" ;;
    status) cmd_status "${run_arg:-$(latest_run)}" ;;
    list) cmd_list "${run_arg:-$(latest_run)}" ;;
    stop) cmd_stop "${run_arg:-$(latest_run)}" ;;
    extract) cmd_extract "${run_arg:-$(latest_run)}" "$@" ;;
    help|-h|--help) usage ;;
    *) usage; exit 1 ;;
esac
