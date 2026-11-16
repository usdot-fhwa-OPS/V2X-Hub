# FLIR capture setup

Runs `../flir_capture.sh` as a systemd service: it starts on boot and on weekdays at 07:00, stops at 18:00 (local time), survives reboots, and keeps 20 GB of packet capture (200 x 100 MB) per run. See `../README.md` for what the capture checks.

## Install

```shell
./install.sh             # asks for the addresses, checks them, installs after confirmation
./install.sh --dry-run   # asks, then prints the service file; installs nothing
./install.sh --yes       # reinstalls with the saved answers, without asking
```

`install.sh` asks the questions below. Press Enter to keep the value in brackets, or type `-` to leave an optional value empty. The answers are saved in `/etc/flir-capture/site.conf` and offered as the defaults next time, so rerun `install.sh` to change them.

| Question | Used for | Suggested |
|---|---|---|
| User the service runs as | runs the capture; added to the `wireshark` group | you |
| FLIR camera IP address | `--camera`: WebSocket, connect checks, ping, RTSP check, capture | (required) |
| Network interface | `--iface`: the interface captured on | the one routing to the camera |
| PoE switch IP, or `none` | `--switch`: pinged and captured next to the camera | (required) |
| Open TCP port on the switch | `--switch-port`: also used as a control host by the probe | none |
| Extra address to reach the switch | added to the interface at each start, for a switch on another subnet (e.g. still on its factory address) | none |
| NetworkManager connection | saves that extra address permanently | the interface's active connection |
| Control hosts | `--control HOST:PORT`, e.g. the gateway or RSU | the default gateway, port 80 |
| This host's own address | `--control` self check, needs an open port such as SSH | this host's address, port 22 |
| Camera firmware, note | recorded in each run's `notes.txt` | none |

Before asking for confirmation it pings the camera and switch and tries a TCP connect to each control host, and warns about any that do not answer. It then:

- installs `tshark` and allows members of the `wireshark` group to capture, and adds the user to that group;
- saves the extra address in the NetworkManager connection, if given;
- generates `/etc/systemd/system/flir-capture.service` from `systemd/flir-capture.service.in`, and installs the timers, the stop service and `/usr/local/bin/flir-capture-in-hours`;
- enables everything and restarts `flir-capture.service`. A run in progress is stopped and a new run starts, unless it is outside the weekday hours.

## Schedule

```mermaid
flowchart LR
    boot["Boot"] --> svc["flir-capture.service"]
    starttimer["flir-capture-start.timer<br/>Mon-Fri 07:00"] --> svc
    svc --> hours{"flir-capture-in-hours:<br/>Mon-Fri 07:00-18:00?"}
    hours -- no --> skip["Skipped"]
    hours -- yes --> addr["Add the extra switch-subnet<br/>address, if set"]
    addr --> wait["Wait up to 3 min<br/>for the camera to answer ping"]
    wait --> start["flir_capture.sh start ..."]
    stoptimer["flir-capture-stop.timer<br/>Mon-Fri 18:00"] --> stopsvc["flir-capture-stop.service"]
    stopsvc --> stop["systemctl stop flir-capture<br/>runs flir_capture.sh stop"]
```

If `flir_capture.sh start` fails, for example because the disk is too full, the service retries every 30 s.

## Example site

The first site this ran at, for reference:

| Role | IP |
|---|---|
| This host | 192.168.55.52 (enp60s0), plus 192.168.0.52/24 to reach the switch |
| FLIR camera | 192.168.55.111 |
| PoE switch | 192.168.0.239 (factory address), port 80 |
| RSU (camera time source) | 192.168.55.20, port 80 |
| Gateway | 192.168.55.1, port 80 |

## Files

- `install.sh`: asks for the site's addresses and installs everything below
- `systemd/flir-capture.service.in`: template for `/etc/systemd/system/flir-capture.service`, the collection run
- `systemd/flir-capture-start.timer`, `flir-capture-stop.timer`, `flir-capture-stop.service` -> `/etc/systemd/system/`
- `flir-capture-in-hours` -> `/usr/local/bin/`: the weekday-hours check used by the service

## Check

`systemctl status flir-capture`, `systemctl list-timers 'flir-capture-*'`, `../flir_capture.sh status`.

## Disk

Every weekday start creates a new run directory of up to 20 GB, and old runs are not deleted. `start` refuses to begin with less than 24 GB free (the service then retries every 30 s), so delete old runs under `../flir_capture_runs/` regularly.
