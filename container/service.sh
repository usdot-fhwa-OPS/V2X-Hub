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
# If in simulation mode, configure and enable the plugins used by CDASim.
if [ "${SIMULATION_MODE^^}" = "TRUE" ] 
then
    # CDASimAdapter listens for ImmediateForward messages on port 5678. Use
    # the per-instance Docker DNS name supplied through V2XHUB_IP so that the
    # destination remains valid when Docker assigns a different container IP.
    immediate_forward_host="${V2XHUB_IP:-127.0.0.1}"
    if [ "$immediate_forward_host" = "0.0.0.0" ]; then
        immediate_forward_host="127.0.0.1"
    fi

    if [[ ! "$immediate_forward_host" =~ ^[A-Za-z0-9._:-]+$ ]]; then
        echo "Invalid V2XHUB_IP for ImmediateForward destination: $immediate_forward_host" >&2
        exit 1
    fi

    printf -v immediate_forward_config '%s' \
        "[{\"name\":\"CDASim Adapter\",\"rsuSpec\":\"RSU4.1\",\"address\":\"${immediate_forward_host}\",\"port\":5678,\"txMode\":\"CONT\",\"signMessages\":false,\"messages\":[{\"tmxType\":\"SPAT-P\",\"sendType\":\"SPAT\",\"psid\":\"0x8002\",\"channel\":183},{\"tmxType\":\"MAP-P\",\"sendType\":\"MAP\",\"psid\":\"0x8002\",\"channel\":183}]}]"

    echo "Configuring simulation plugins with ImmediateForward destination ${immediate_forward_host}:5678"
    tmxctl --plugin ImmediateForward --set \
        --key ImmediateForwardConfigurations \
        --value "$immediate_forward_config"
    tmxctl --plugin SPAT --set --key SPAT_Mode --value SPAT
    tmxctl --plugin MessageReceiver --set --key Port --value 8765
    tmxctl --plugin MessageReceiver --set --key RouteJ2735 --value true

    echo "Enabling V2X-Hub plugins for CDASim integration"
    tmxctl --plugin SPAT --enable
    tmxctl --plugin MAP --enable
    tmxctl --plugin ImmediateForward --enable
    tmxctl --plugin MessageReceiver --enable
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
