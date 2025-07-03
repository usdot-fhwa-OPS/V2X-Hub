#!/bin/bash

# Initialize Docker Environment for V2X Hub
./initialize_docker_environment.sh
# Initialize secrets
./initialize_secrets.sh
# Install Docker
./install_docker.sh
# Run V2X Hub
./run_v2xhub.sh
