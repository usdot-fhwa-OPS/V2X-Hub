#!/usr/bin/env bash
# Reinstall the FLIR capture setup (V2X-Hub PR 991) on this machine.
# Lives in V2X-Hub/src/v2i-hub/FLIRCameraDriverPlugin/scripts/flir_capture_setup; the service runs
# ../flir_capture.sh from ~/workspace/V2X-Hub as user "vug"
# (edit systemd/flir-capture.service if the path, user, interface or IPs differ).
set -euo pipefail
cd "$(dirname "$0")"
sudo apt-get install -y tshark
sudo usermod -aG wireshark vug
# Switch-subnet address on the camera interface (switch 192.168.0.239)
sudo nmcli con mod share-eth +ipv4.addresses 192.168.0.52/24 || echo "warning: set 192.168.0.52/24 on the camera interface yourself"
sudo install -m 755 flir-capture-in-hours /usr/local/bin/flir-capture-in-hours
sudo install -m 644 systemd/* /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable flir-capture.service flir-capture-start.timer flir-capture-stop.timer
sudo systemctl start flir-capture-start.timer flir-capture-stop.timer
sudo systemctl start flir-capture.service   # skipped automatically outside Mon-Fri 07:00-18:00
systemctl list-timers 'flir-capture-*' --no-pager
