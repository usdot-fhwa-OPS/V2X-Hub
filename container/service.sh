#!/bin/bash
wait-for-it.sh 127.0.0.1:3306

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
# Enable required plugins for TelematicRSU Unit
if [ "${IS_TRU^^}" = "TRUE" ]
then
    echo "Enabling MessageReceiver RSUHealthMonitor TelematicsBridgePlugin for RSU data streaming!"
    tmxctl --plugin MessageReceiver --enable
    tmxctl --plugin RSUHealthMonitor --enable
    tmxctl --plugin TelematicsBridgePlugin --enable
fi
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
# Start Tmx Core
tmxcore
