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

# Update permissions for tmx logs created by plugins
sudo chmod -R 777 ./logs

# Create V2X Hub user
source .env
./add_v2xhub_user.sh "$V2XHUB_VERSION"

chromium-browser localhost > /dev/null 2>&1 &

echo "V2X Hub Deployment Complete."
