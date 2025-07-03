# #!/bin/bash
echo "Running V2X Hub..."
# Install necessary and useful apps
sudo apt update -y 
sudo apt-get install chromium-browser -y
sudo docker compose up -d


# Update permissions for tmx logs created by plugins
sudo chmod -R 777 ./logs

# Create V2X Hub user
source .env
./add_v2xhub_user.sh "$V2XHUB_VERSION"

chromium-browser --ignore-certificate-errors localhost > /dev/null 2>&1 &

echo "V2X Hub Deployment Complete."