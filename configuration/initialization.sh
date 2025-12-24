#!/bin/bash
set -e

# Initialize Docker Environment for V2X Hub
./initialize_docker_environment.sh
# Install Docker
MIN_VERSION="2.1.0" # Example version, adjust as needed

echo "Checking for docker compose installation and version..."

# Check if the 'docker compose' command exists (without the hyphen)
if docker compose version &>/dev/null; then
    # If the command exists, get the version
    COMPOSE_VERSION=$(docker compose version --short)
    echo "Docker Compose (V2) is installed. Version: $COMPOSE_VERSION"

    # Compare the installed version with the minimum required version
    if dpkg --compare-versions "$COMPOSE_VERSION" "ge" "$MIN_VERSION"; then
        echo "Docker Compose version $COMPOSE_VERSION meets or exceeds the minimum required version $MIN_VERSION."
    else
        echo "Docker Compose version $COMPOSE_VERSION is below the minimum required version $MIN_VERSION."
        echo "Reinstalling Docker"
        ./install_docker.sh
    fi
else
   ./install_docker.sh
fi
read -p "Would you like to generated local browser trusted certificates? (Y/N): " user_choice
# Convert the input to lowercase for case-insensitive comparison
user_choice_lower=$(echo "$user_choice" | tr '[:upper:]' '[:lower:]')
# Conditionally execute logic based on user input
if [[ "$user_choice_lower" == "y" ]]; then
    echo "Using locally trusted browser certificates requires setting up ssl_datavolume to map to local directory. Please see docker-compose.yml volume section for instruction!"
    ./generate_local_trusted_certificates.sh
fi

# Run V2X Hub
./run_v2xhub.sh
