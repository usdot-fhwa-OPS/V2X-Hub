# #!/bin/bash
echo "Running V2X Hub..."
# Check for Chromium and install if not already installed.
dpkg -s chromium-browser >/dev/null 2>&1
if [ $? -eq 0 ]; then
  echo "Chromium is installed!"
else
  echo "Chromium is not installed. Installing Chromium ..."
  sudo apt update -y 
  sudo apt-get install chromium-browser -y
fi

sudo docker compose up -d


# Update permissions for tmx logs created by plugins
sudo chmod -R 777 ./logs

# Create V2X Hub user
source .env
./add_v2xhub_user.sh "$V2XHUB_VERSION"

chromium-browser --ignore-certificate-errors localhost > /dev/null 2>&1 &

echo "V2X Hub Deployment Complete."
