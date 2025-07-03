#!/bin/bash

# Default values for the variables
PORT_DRAYAGE_ENABLED_DEFAULT="FALSE"
INFRASTRUCTURE_ID_DEFAULT="rsu_1234"
INFRASTRUCTURE_NAME_DEFAULT="East Intersection"
V2XHUB_IP_DEFAULT="127.0.0.1"
SIMULATION_MODE_DEFAULT="FALSE"
SIMULATION_IP_DEFAULT="127.0.0.1"
SENSOR_JSON_FILE_PATH_DEFAULT="/var/www/plugins/MAP/sensors.json"
COMPOSE_PROFILES=""

echo "Setting up the environment..."

# Repository URL
repo_url_latest="https://api.github.com/repos/usdot-fhwa-OPS/V2X-Hub/releases/latest"

# Getting the latest release information using curl
release_info=$(curl -sSL $repo_url_latest)

# Parsing the JSON response to get the tag_name (version) of the latest release
latest_version=$(echo "$release_info" | grep -o '"tag_name": *"[^"]*"' | cut -d '"' -f 4)

# Fetching all tags from Git repository
tags=$(git ls-remote --tags https://github.com/usdot-fhwa-OPS/V2X-Hub.git | awk -F/ '{print $3}' | sort -V)

# Remove curly braces, Properties found, duplicate entries, and show only versions above 7.0
updated_tags=$(echo "$tags" | sed 's/\^{}//;s/^v//' | grep -v '^Properties_Found$' | awk '!seen[$0]++ && $1 >= "7.0"')

# Displaying all available versions
echo "Note: V2X-Hub multi architecture deployments only work for the versions 7.0 and above."
echo "Available versions:"
echo "$updated_tags"

# select a version or accept the latest version as default
read -r -p "Enter V2X-Hub Version (choose from the above, or press Enter to use the latest version $latest_version): " chosen_version
V2XHUB_VERSION=${chosen_version:-$latest_version}

# Enable Port Drayage functionality
read -r -p "Enable Port Drayage functionality (TRUE/FALSE, or press Enter to use default as $PORT_DRAYAGE_ENABLED_DEFAULT): " PORT_DRAYAGE_ENABLED
PORT_DRAYAGE_ENABLED=${PORT_DRAYAGE_ENABLED:-$PORT_DRAYAGE_ENABLED_DEFAULT}
if [[ $PORT_DRAYAGE_ENABLED == "TRUE" ]]; then
    echo "Activating port_drayage profile."
    COMPOSE_PROFILES="port_drayage"
fi
# Infrastructure id
read -r -p "Enter Infrastructure id (or press Enter to use default as $INFRASTRUCTURE_ID_DEFAULT): " INFRASTRUCTURE_ID
INFRASTRUCTURE_ID=${INFRASTRUCTURE_ID:-$INFRASTRUCTURE_ID_DEFAULT}

# Infrastructure name
read -r -p "Enter Infrastructure name (or press Enter to use default as $INFRASTRUCTURE_NAME_DEFAULT): " INFRASTRUCTURE_NAME
INFRASTRUCTURE_NAME=${INFRASTRUCTURE_NAME:-$INFRASTRUCTURE_NAME_DEFAULT}

# V2XHub IP
read -r -p "Enter V2XHub IP (or press Enter to use default as $V2XHUB_IP_DEFAULT): " V2XHUB_IP
V2XHUB_IP=${V2XHUB_IP:-$V2XHUB_IP_DEFAULT}

# Simulation Mode
read -r -p "Simulation Mode (TRUE/FALSE, or press Enter to use default as $SIMULATION_MODE_DEFAULT): " SIMULATION_MODE
SIMULATION_MODE=${SIMULATION_MODE:-$SIMULATION_MODE_DEFAULT}

# In Simulation Mode
if [[ $SIMULATION_MODE == "TRUE" ]]; then
    # Simulation IP
    read -r -p "Enter Simulation IP (or press Enter to use default is $SIMULATION_IP_DEFAULT): " SIMULATION_IP
    SIMULATION_IP=${SIMULATION_IP:-$SIMULATION_IP_DEFAULT}

    # Sensor Configuration File Path
    read -r -p "Enter Sensor Configuration File Path (or press Enter to use default as $SENSOR_JSON_FILE_PATH_DEFAULT): " SENSOR_JSON_FILE_PATH
    SENSOR_JSON_FILE_PATH=${SENSOR_JSON_FILE_PATH:-$SENSOR_JSON_FILE_PATH_DEFAULT}
fi

echo "WARNING: This will overwrite the existing .env file if it exists."
read -r -p "Are you sure you want to continue? (Y/N): " overwrite_confirm
if [[ "$overwrite_confirm" =~ [yY](es)* ]]; then
    # Write to .env file
    cat <<EOF > .env
V2XHUB_VERSION=$V2XHUB_VERSION
INFRASTRUCTURE_ID=$INFRASTRUCTURE_ID
INFRASTRUCTURE_NAME=$INFRASTRUCTURE_NAME
V2XHUB_IP=$V2XHUB_IP
SIMULATION_MODE=$SIMULATION_MODE
COMPOSE_PROFILES=$COMPOSE_PROFILES
EOF

    # Adding Simulation IP and Sensor Path if Simulation Mode is TRUE
    if [[ $SIMULATION_MODE == "TRUE" ]]; then
        echo "SIMULATION_IP=$SIMULATION_IP" >> .env
        echo "SENSOR_JSON_FILE_PATH=$SENSOR_JSON_FILE_PATH" >> .env
    fi
else
    echo "Aborting. No changes were made to the .env file."
fi

directory=$(pwd)
mysqlDir="$directory/mysql"


# Install necessary and useful apps
sudo apt update -y 
sudo apt-get install chromium-browser -y

# Make passwords for mysql
mkdir -p secrets && cd secrets || return # SC2164 - Use return in case cd fails 

# Creates password files where user inputs password
FILE1=mysql_root_password.txt
FILE2=mysql_password.txt
if test -f "$FILE1"; then
    echo "$FILE1 exists."
else
    read -r -p "enter password for the mysql_root_password: " sql_root_pass # SC2162 - read without -r will mangle backslashes 
    echo "$sql_root_pass" > sql_root_pass.txt
    # Remove endline characters from password files
    tr -d '\n' <sql_root_pass.txt> mysql_root_password.txt && rm sql_root_pass.txt
fi

if test -f "$FILE2"; then
    echo "$FILE2 exists."
else
    read -r -p "enter password for mysql_password: " sql_pass
    echo "$sql_pass" > sql_pass.txt
    # Remove endline characters from password files
    tr -d '\n' <sql_pass.txt> mysql_password.txt && rm sql_pass.txt
fi

# AMD64 initialization
cd "$directory" || return # return in case cd fails
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


# Update permissions for tmx logs created by plugins
sudo chmod -R 777 ./logs

# Create V2X Hub user
cd "$mysqlDir" || return # return in case cd fails
./add_v2xhub_user.bash "$V2XHUB_VERSION"

chromium-browser --ignore-certificate-errors localhost > /dev/null 2>&1 &
