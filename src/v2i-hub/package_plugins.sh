#!/bin/bash
set -e

PLUGINS=(
    CARMACloudPlugin
    CARMAStreetsPlugin
    CommandPlugin
    CswPlugin
    DmsPlugin
    ImmediateForwardPlugin
    LocationPlugin
    MapPlugin
    MessageLoggerPlugin
    MessageReceiverPlugin
    ODEForwardPlugin
    PedestrianPlugin
    PortDrayagePlugin
    PreemptionPlugin
    RtcmPlugin
    SpatPlugin
    TimPlugin
)

# package up all plugins
for PLUGIN in "${PLUGINS[@]}"; do
    ln -s ../bin "$PLUGIN"/bin
    zip "$PLUGIN".zip "$PLUGIN"/bin/"$PLUGIN" "$PLUGIN"/manifest.json
done