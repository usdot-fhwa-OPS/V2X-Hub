#!/bin/bash
wait-for-it.sh 127.0.0.1:3306

# Add stol apt repository libraries to path for tmxcore
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/opt/carma/lib/
for plugin in /usr/local/plugins/*.zip; do
    echo "Installing plugin $plugin"
    tmxctl --plugin-install "$plugin"
done

echo "TelematicBridge Configuration:"
echo "  RSU_CONFIG_PATH: $RSU_CONFIG_PATH"
echo "  NATS_URL: $NATS_URL"
echo "  IS_TRU: $IS_TRU"

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
tmxctl --plugin MessageReceiver --enable
tmxctl --plugin RSUHealthMonitor --enable
tmxctl --plugin TelematicBridge --enable

# Wait for tmxcore process
wait $TMXCORE_PID
