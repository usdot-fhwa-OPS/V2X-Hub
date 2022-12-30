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
    ERVCloudForwardingPlugin
)

# package up all plugins
for PLUGIN in "${PLUGINS[@]}"; do
    # force in case it exists already
    ln --force -s ../bin "$PLUGIN"/bin
    zip "$PLUGIN".zip "$PLUGIN"/bin/"$PLUGIN" "$PLUGIN"/manifest.json
done