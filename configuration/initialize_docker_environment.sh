#!/bin/bash
# Script to initialize docker environment for V2X Hub.

set -e

interactive=false
show_help() {
  echo "Usage: $0 -i "
  echo ""
  echo "  -i, --interactive     Run the script in interactive mode. If present, the script will prompt the user for input. If not present, the script will use default values or values from environment variables."
  echo "  -h, --help            Show this help message and exit"
  echo ""
  echo "If arguments are not provided, the script will prompt interactively."
}
# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    -i|--interactive)
      interactive=true
      echo "Running in interactive mode. The script will prompt for input values."
      shift
      ;;
    -h|--help)
      show_help
      exit 0
      ;;
    --*)
      echo "Unknown option: $1"
      echo ""
      show_help
      exit 1
      ;;
  esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ "$interactive" == true ]]; then
  

    # Default values for the variables
    PORT_DRAYAGE_ENABLED_DEFAULT="FALSE"
    INFRASTRUCTURE_ID_DEFAULT="rsu_1234"
    INFRASTRUCTURE_NAME_DEFAULT="East Intersection"
    V2XHUB_IP_DEFAULT="0.0.0.0"
    SIMULATION_MODE_DEFAULT="FALSE"
    SENSOR_JSON_FILE_PATH_DEFAULT="/var/www/download/sensors.json"
    V2XHUB_VOLUME_PATH_DEFAULT="/tmp/v2xhub_data/"
    COMPOSE_PROFILES=""

    echo "Initializing Docker Environment for V2X Hub..."
    # Retrieve available release candidates
    release_candidates=$(git branch -r | grep 'origin/release/' | sed 's|origin/||' | sed 's/release\//release-/g')
    echo "Available Release Candidates (Only intended for use during release-testing):"
    echo "$release_candidates"
    # Repository URL
    repo_url_latest="https://api.github.com/repos/usdot-fhwa-OPS/V2X-Hub/releases/latest"

    # Getting the latest release information using curl
    release_info=$(curl -sSL $repo_url_latest)

    # Parsing the JSON response to get the tag_name (version) of the latest release
    latest_version=$(echo "$release_info" | grep -o '"tag_name": *"[^"]*"' | cut -d '"' -f 4)

    # Fetching all tags from Git repository
    tags=$(git ls-remote --tags https://github.com/usdot-fhwa-OPS/V2X-Hub.git | awk -F/ '{ printf "  %s\n", $3 }' | sort -V)
    # Remove curly braces, Properties found, duplicate entries, and tag that starts with v. and show only versions above 7.0
    updated_tags=$(echo "$tags" | sed 's/\^{}//;s/^v//' | grep -vE 'Properties_Found|v.*'  | awk '!seen[$0]++ && $1 >= "7.0"')


    # Displaying all available versions
    echo "Note: V2X-Hub multi architecture deployments only work for the versions 7.0 and above."
    echo "Available Release versions:"
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

    # V2X Hub Volume Path
    read -r -p "Enter an absolute file path to store V2X Hub Volumes  (or press Enter to use default as $V2XHUB_VOLUME_PATH_DEFAULT): " V2XHUB_VOLUME_PATH
    V2XHUB_VOLUME_PATH=${V2XHUB_VOLUME_PATH:-$V2XHUB_VOLUME_PATH_DEFAULT}
    
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
        read -r -p "Enter Simulation IP (loop back addresses will not work and redirect traffic to containers virtual network ip): " SIMULATION_IP
        SIMULATION_IP=${SIMULATION_IP}

        # Sensor Configuration File Path
        read -r -p "Enter Sensor Configuration File Path (or press Enter to use default as $SENSOR_JSON_FILE_PATH_DEFAULT): " SENSOR_JSON_FILE_PATH
        SENSOR_JSON_FILE_PATH=${SENSOR_JSON_FILE_PATH:-$SENSOR_JSON_FILE_PATH_DEFAULT}
    fi
    # MySQL (IVP) User Password
    read -r -s -p "MYSQL PASSWORD (password for configuration database): " MYSQL_PASSWORD
    echo
    # V2X Hub Username
    read -r -p "V2X Hub Admin Username (or press Enter to use default as v2xadmin): " V2XHUB_USERNAME
    V2XHUB_USERNAME=${V2XHUB_USERNAME:-v2xadmin}
    echo "Password must be 8-12 charcters, and contain at least one of each of the following: uppercase letter, lowercase letter, number, and symbol."

    # V2X Hub Password
    read -r -s -p "V2X Hub Admin Password (input will be hidden): " V2XHUB_PASSWORD
    # Get length of the password
    PASS_LENGTH=$(echo "$V2XHUB_PASSWORD" | wc -c)
    # Validate password complexity
    if [ $PASS_LENGTH -ge 8 ] && echo "$V2XHUB_PASSWORD" | grep -q "[a-z]" && echo "$V2XHUB_PASSWORD" | grep -q "[A-Z]" && echo "$V2XHUB_PASSWORD" | grep -q "[0-9]" && ( echo "$V2XHUB_PASSWORD" | grep -q "[\$\!\.\+_\*@\#\^%\?~]" || echo "$V2XHUB_PASSWORD" | grep -q "[-]" ); then
        echo
        # Confirm password
        read -r -s -p "Confirm password: " CONF_PASS
        while [ "$CONF_PASS" != "$V2XHUB_PASSWORD" ]; do
            echo
            read -r -s -p "Passwords do not match. Please re-enter password: " CONF_PASS
        done
        echo "VALID PASSWORD"
    else
        echo "INVALID PASSWORD"
        echo "Password must be 8-12 charcters, and contain at least one of each of the following: uppercase letter, lowercase letter, number, and symbol"
        exit 1
    fi
    echo "WARNING: This will overwrite the existing .env file if it exists."
    read -r -p "Are you sure you want to continue? (Y/N): " overwrite_confirm
    if [[ "$overwrite_confirm" =~ [yY](es)* ]]; then
        # Write to .env file
        ENV_FILE_CONTENT="
        V2XHUB_VERSION=$V2XHUB_VERSION
        INFRASTRUCTURE_ID=$INFRASTRUCTURE_ID
        INFRASTRUCTURE_NAME=$INFRASTRUCTURE_NAME
        V2XHUB_IP=$V2XHUB_IP
        SIMULATION_MODE=$SIMULATION_MODE
        COMPOSE_PROFILES=$COMPOSE_PROFILES
        SENSOR_JSON_FILE_PATH=$SENSOR_JSON_FILE_PATH
        MYSQL_PASSWORD=$MYSQL_PASSWORD
        V2XHUB_USERNAME=$V2XHUB_USERNAME
        V2XHUB_PASSWORD=$V2XHUB_PASSWORD
        SIMULATION_IP=$SIMULATION_IP 
        V2XHUB_VOLUME_PATH=$V2XHUB_VOLUME_PATH
        "
        echo "$ENV_FILE_CONTENT" > "$SCRIPT_DIR/.env"
    else
        echo "Aborting. No changes were made to the .env file."
    fi
# If in non-interactive mode, check if .env file exists and fail if not
elif [[ -f "$SCRIPT_DIR/.env" ]]; then
    echo "Using existing .env file."
else
    echo "Error: .env file not found and no interactive input provided."
    exit 1
fi
# Read .env file and create volume directories if they do not exist
source "$SCRIPT_DIR/.env"
echo "Creating volume directories at ${V2XHUB_VOLUME_PATH}/download/ ${V2XHUB_VOLUME_PATH}/logs/ ${V2XHUB_VOLUME_PATH}/ssl/"
mkdir -m 755 -p ${V2XHUB_VOLUME_PATH}/download/ ${V2XHUB_VOLUME_PATH}/logs/ ${V2XHUB_VOLUME_PATH}/ssl/
echo "Docker Environment Initialization Complete."
