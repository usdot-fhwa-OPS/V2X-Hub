#!/bin/bash
set -e
echo "Running V2X Hub..."

if ! command -v chromium-browser &>/dev/null; then
  echo "chromium-browser not found, install chromium-browser"
  sudo apt update
  sudo apt install chromium-browser -y
fi
# Start V2X Hub
sudo docker compose up -d

# Create and update permissions for tmx logs directory
mkdir -p ./logs
sudo chmod -R 777 ./logs


chromium-browser localhost > /dev/null 2>&1 &

echo "V2X Hub Deployment Complete."
