# FLIR capture setup

Runs `../flir_capture.sh` as a systemd service: it starts on boot and on weekdays at 07:00, stops at 18:00 (local time), survives reboots, and keeps 20 GB of packet capture (200 x 100 MB) per run. See `../README.md` for what the capture checks.

## Install

```shell
./install.sh             # asks for the addresses, checks them, installs after confirmation
./install.sh --dry-run   # asks, then prints the service file; installs nothing
./install.sh --yes       # reinstalls with the saved answers, without asking
```

`install.sh` asks the questions below, showing the default in brackets (`[none]` for an optional value left empty). Press Enter to keep it, or type `-` or `none` to leave an optional value empty. The answers are saved in `/etc/flir-capture/site.conf` and become the defaults next time, so rerun `install.sh` to change them.

```text
User the service runs as (needs to read .../scripts) [vug]:
FLIR camera IP address (WebSocket on port 80) [192.168.55.183]: 192.168.55.111
Network interface that reaches the camera (captured on) [enp60s0]:
PoE switch IP, pinged next to the camera (or another device on the same switch; none to skip): 192.168.0.239
  192.168.0.239 is not on a subnet enp60s0 is attached to. If the switch only has its factory address,
  give this host an extra address on the switch's subnet, e.g. 192.168.0.52/24 for a switch at 192.168.0.239.
Extra address to add to enp60s0 to reach the switch, ADDRESS/PREFIX [none]: 192.168.0.52/24
Control hosts with an open TCP port, e.g. the gateway or RSU: HOST[:PORT], port 80 if left out, space-separated [192.168.55.1:80]: 192.168.55.1 192.168.55.20
Note recorded with each run: firmware, switch, port, cable, time source [none]: firmware V2.05, switch A port 3
```

| Question | Used for | Default the first time |
|---|---|---|
| User the service runs as | runs the capture; added to the `wireshark` group | you |
| FLIR camera IP address | `--camera`: WebSocket (port 80), connect checks, ping, capture | 192.168.55.183 (`flir_capture.sh`'s default) |
| Network interface | `--iface`: the interface captured on | the one routing to the camera |
| PoE switch IP, or `none` | `--switch`: pinged and captured next to the camera | none, an answer is required |
| Extra address to reach the switch | only asked when the switch is on another subnet (e.g. still on its factory address); added to the interface at each start and saved in its NetworkManager connection | none |
| Control hosts, `HOST[:PORT]` | `--control`, e.g. the gateway or RSU; port 80 if left out | the default gateway, port 80 |
| Note | recorded in each run's `notes.txt`: firmware, switch, port, cable | none |

This host's own address is checked by the probe automatically, so it is not asked.

Before asking for confirmation it pings the camera and switch, tries a TCP connect to the camera's WebSocket port and each control host, and checks the free disk space, and warns about any problem. It then:

- installs `tshark` and allows members of the `wireshark` group to capture, and adds the user to that group;
- saves the extra address in the interface's NetworkManager connection, if one is given;
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
| PoE switch | 192.168.0.239 (factory address) |
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
