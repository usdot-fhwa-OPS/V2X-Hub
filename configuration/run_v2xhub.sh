#!/bin/bash
echo "Running V2X Hub..."
# Check for Chromium and install if not already installed.
REQUIRED_PKG="chromium-browser"
PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $REQUIRED_PKG|grep "install ok installed")
echo Checking for $REQUIRED_PKG: $PKG_OK
if [ "" = "$PKG_OK" ]; then
  echo "No $REQUIRED_PKG found. Installing $REQUIRED_PKG."
  sudo apt-get update
  sudo apt-get --yes install $REQUIRED_PKG
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
