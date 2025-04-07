#!/bin/bash
wait-for-it.sh 127.0.0.1:3306

cd /var/log/tmx
# Add stol apt repository libraries to path for tmxcore
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/opt/carma/lib/
for plugin in /usr/local/plugins/*.zip; do
    echo "Installing plugin $plugin"
    tmxctl --plugin-install "$plugin"
done

# command plugin must always be enabled
tmxctl --plugin CommandPlugin --enable
# If in simulation mode, enable SimulationAdapter
if [ "${SIMULATION_MODE^^}" = "TRUE" ] 
then
    echo "Enabling CDASim Adapter for Simulation Integration!"
    tmxctl --plugin CDASimAdapter --enable
fi
tmxcore