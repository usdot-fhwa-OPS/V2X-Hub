#!/bin/bash
set -e
echo "Installing Docker on Ubuntu..."
echo "Removing any existing docker packages ..."
set +e
# Install Docker on Ubuntu
for pkg in docker.io docker-doc docker-compose podman-docker containerd runc; do sudo apt-get remove $pkg; done
# Add Docker's official GPG key:
set -e
echo "Adding docker apt package repository ... "
sudo apt update
sudo apt install -y --no-install-recommends ca-certificates curl
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/$(. /etc/os-release && echo "$ID")/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc

# Add the repository to Apt sources:
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/$(. /etc/os-release && echo "$ID") \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
echo "Installing docker and docker compose ..."
sudo apt update
sudo apt install -y --no-install-recommends docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin  

echo "Docker Installation Complete."
