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
)

# package up all plugins
for PLUGIN in "${PLUGINS[@]}"; do
    # remove the link if it exists already
    rm -f "$PLUGIN"/bin
    ln -s ../bin "$PLUGIN"/bin
    zip "$PLUGIN".zip "$PLUGIN"/bin/"$PLUGIN" "$PLUGIN"/manifest.json
done