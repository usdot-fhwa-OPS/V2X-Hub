#!/bin/bash
directory=$(pwd)
mysqlDir="$directory/mysql"

# Update and upgrade commands to update linux OS
sudo apt update -y && sudo apt upgrade -y

# Install necessary and useful apps
sudo apt-get install chromium-browser -y

# Make passwords for mysql
mkdir -p secrets && cd secrets

# Creates password files where user inputs password
FILE1=mysql_root_password.txt
FILE2=mysql_password.txt
if test -f "$FILE1"; then
    echo "$FILE1 exists."
else
    read -p "enter password for the mysql_root_password: " sql_root_pass
    echo "$sql_root_pass" > sql_root_pass.txt
    # Remove endline characters from password files
    tr -d '\n' <sql_root_pass.txt> mysql_root_password.txt && rm sql_root_pass.txt
fi

if test -f "$FILE2"; then
    echo "$FILE2 exists."
else
    read -p "enter password for mysql_password: " sql_pass
    echo "$sql_pass" > sql_pass.txt
    # Remove endline characters from password files
    tr -d '\n' <sql_pass.txt> mysql_password.txt && rm sql_pass.txt
fi
# TODO VH-1303 Allow for version and configuration selection in initialization script

# AMD64 initialization
cd $directory
for pkg in docker.io docker-doc docker-compose podman-docker containerd runc; do sudo apt-get remove $pkg; done
# Add Docker's official GPG key:
sudo apt-get update
sudo apt-get install ca-certificates curl
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/$(. /etc/os-release && echo "$ID")/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc

# Add the repository to Apt sources:
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/$(. /etc/os-release && echo "$ID") \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt-get update
sudo apt-get -y install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
sudo docker compose up -d

# Create V2X Hub user
cd $mysqlDir
./add_v2xhub_user.bash

chromium-browser "http://127.0.0.1" > /dev/null 2>&1 &
chromium-browser "https://127.0.0.1:19760" > /dev/null 2>&1 &
