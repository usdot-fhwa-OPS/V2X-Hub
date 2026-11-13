# FLIR capture setup

Site setup that runs `../flir_capture.sh` (V2X-Hub PR 991) as a systemd service
on weekdays 07:00-18:00
local time, survives reboots, keeps 20 GB of packet capture (200 x 100 MB) per run.

| Role | IP |
|---|---|
| This host | 192.168.55.52 (enp60s0), plus 192.168.0.52/24 to reach the switch |
| FLIR camera | 192.168.55.111 |
| PoE switch | 192.168.0.239 (default IP), port 80 |
| RSU (camera time source) | 192.168.55.20, port 80 |
| Gateway | 192.168.55.1, port 80 |

Files (the installed copies are the live ones; reinstall after editing here):
- `systemd/flir-capture.service` -> /etc/systemd/system/ : the collection run
- `systemd/flir-capture-start.timer`, `flir-capture-stop.timer`, `flir-capture-stop.service` -> /etc/systemd/system/
- `flir-capture-in-hours` -> /usr/local/bin/ : weekday-hours check used by the service
- NetworkManager connection `share-eth`: extra address 192.168.0.52/24

Reinstall: `./install.sh`

Check: `systemctl status flir-capture`, `systemctl list-timers 'flir-capture-*'`,
`../flir_capture.sh status`.
