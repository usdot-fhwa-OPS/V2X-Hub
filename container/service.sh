#!/bin/bash

# Add stol apt repository libraries to path for tmxcore
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/opt/carma/lib/
for plugin in /usr/local/plugins/*.zip; do
    echo "Installing plugin $plugin"
    tmxctl --plugin-install "$plugin"
done
# Generate self-signed certificates if they do not already exist
/home/V2X-Hub/container/generate_certificates.sh
# command plugin must always be enabled
tmxctl --plugin CommandPlugin --enable
# If in simulation mode, enable SimulationAdapter
if [ "${SIMULATION_MODE^^}" = "TRUE" ] 
then
    echo "Enabling CDASim Adapter for Simulation Integration!"
    tmxctl --plugin CDASimAdapter --enable
fi
# If V2XHUB_USER and V2XHUB_PASSWORD are set, create the user
if [ -n "$V2XHUB_USERNAME" ] && [ -n "$V2XHUB_PASSWORD" ]; then
    echo "Creating V2X Hub Admin User: $V2XHUB_USERNAME"
    # Add V2XHub admin user (Will not add if user already exists)
    tmxctl --user-add --username "$V2XHUB_USERNAME" --password "$V2XHUB_PASSWORD" --access-level 3
fi

# If V2X HUB Save State file exists at SAVE_STATE_FILE_PATH then load it to restore the state of the system
# and SAVE_STATE_PASS_PHRASE is set, then attempt to load it
if [ -f "$SAVE_STATE_FILE_PATH" ] && [ -n "$SAVE_STATE_PASS_PHRASE" ]; then
    echo "Loading V2X Hub Save State from $SAVE_STATE_FILE_PATH"
    tmxctl --upload-state "$SAVE_STATE_FILE_PATH" --passphrase "$SAVE_STATE_PASS_PHRASE"
    echo "V2X Hub Save State loaded successfully!"
fi

# Start Tmx Core
tmxcore
