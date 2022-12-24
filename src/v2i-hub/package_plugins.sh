#!/bin/bash
set -e

PLUGINS=(
    CARMACloudPlugin
    CARMAStreetsPlugin
    CommandPlugin
    CswPlugin
    DmsPlugin
    DsrcImmediateForwardPlugin
    LocationPlugin
    MapPlugin
    MessageLoggerPlugin
    MessageReceiverPlugin
    ODELoggerPlugin
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
    ln -s ../bin "$PLUGIN"/bin
    zip "$PLUGIN".zip "$PLUGIN"/bin/"$PLUGIN" "$PLUGIN"/manifest.json
done