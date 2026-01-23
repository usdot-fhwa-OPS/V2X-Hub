#!/bin/bash
wait-for-it.sh 127.0.0.1:3306

# Add stol apt repository libraries to path for tmxcore
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/opt/carma/lib/

# Set TelematicBridge configuration environment variables
export RSU_CONFIG_PATH="${RSU_CONFIG_PATH:-/workspace/configuration/rsuConfigs.json}"
export INFRASTRUCTURE_ID="${INFRASTRUCTURE_ID:-Unit001}"
export INFRASTRUCTURE_NAME="${INFRASTRUCTURE_NAME:-TestUnit}"
export NATS_URL="${NATS_URL:-nats://localhost:4222}"

echo "TelematicBridge Configuration:"
echo "  RSU_CONFIG_PATH: $RSU_CONFIG_PATH"
echo "  INFRASTRUCTURE_ID: $INFRASTRUCTURE_ID"
echo "  INFRASTRUCTURE_NAME: $INFRASTRUCTURE_NAME"
echo "  NATS_URL: $NATS_URL"

for plugin in /usr/local/plugins/*.zip; do
    echo "Installing plugin $plugin"
    tmxctl --plugin-install "$plugin"
done
# # Generate self-signed certificates if they do not already exist
# /home/V2X-Hub/container/generate_certificates.sh

# If V2XHUB_USER and V2XHUB_PASSWORD are set, create the user
# if [ -n "$V2XHUB_USERNAME" ] && [ -n "$V2XHUB_PASSWORD" ]; then
#     echo "Creating V2X Hub Admin User: $V2XHUB_USERNAME"
#     # Add V2XHub admin user (Will not add if user already exists)
#     tmxctl --user-add --username "$V2XHUB_USERNAME" --password "$V2XHUB_PASSWORD" --access-level 3
# fi

# Start Tmx Core in background
echo "Starting TMX Core..."
tmxcore &
TMXCORE_PID=$!

# Wait for tmxcore to be ready
echo "Waiting for TMX Core to initialize..."
sleep 5

# Enable required plugins for TelematicRSU Unit
echo "Configuring plugins for RSU data streaming..."

# Now enable the required plugins
echo "Enabling MessageReceiver RSUHealthMonitor TelematicBridge..."
# tmxctl --plugin MessageReceiver --enable
tmxctl --plugin RSUHealthMonitor --enable
tmxctl --plugin TelematicBridge --enable

# Wait for tmxcore process
wait $TMXCORE_PID
