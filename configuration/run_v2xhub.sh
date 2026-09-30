#!/bin/bash
set -e
echo "Running V2X Hub..."

if ! command -v chromium-browser &>/dev/null; then
  echo "chromium-browser not found, checking for chromium..."

  if ! command -v chromium &>/dev/null; then
    echo "chromium not found, installing chromium..."
    sudo apt update
    sudo apt install chromium-browser -y || sudo apt install chromium -y
  else
    echo "chromium is already installed."
  fi
else
  echo "chromium-browser is already installed."
fi

# Start V2X Hub
sudo docker compose up -d

# Create and update permissions for tmx logs directory
mkdir -p ./logs
sudo chmod -R 777 ./logs


chromium-browser localhost > /dev/null 2>&1 &

echo "V2X Hub Deployment Complete."
