#!/bin/sh

# exit on errors
set -e

# package up all plugins
ln -s ../bin CommandPlugin/bin
zip CommandPlugin.zip CommandPlugin/bin/CommandPlugin CommandPlugin/manifest.json
ln -s ../bin CswPlugin/bin
zip CswPlugin.zip CswPlugin/bin/CswPlugin CswPlugin/manifest.json
ln -s ../bin DmsPlugin/bin
zip DmsPlugin.zip DmsPlugin/bin/DmsPlugin DmsPlugin/manifest.json
ln -s ../bin DsrcImmediateForwardPlugin/bin
zip DsrcImmediateForwardPlugin.zip DsrcImmediateForwardPlugin/bin/DsrcImmediateForwardPlugin DsrcImmediateForwardPlugin/manifest.json
ln -s ../bin LocationPlugin/bin
zip LocationPlugin.zip LocationPlugin/bin/LocationPlugin LocationPlugin/manifest.json
ln -s ../bin MapPlugin/bin
zip MapPlugin.zip MapPlugin/bin/MapPlugin MapPlugin/manifest.json
ln -s ../bin MessageReceiverPlugin/bin
zip MessageReceiverPlugin.zip MessageReceiverPlugin/bin/MessageReceiverPlugin MessageReceiverPlugin/manifest.json
ln -s ../bin RtcmPlugin/bin
zip RtcmPlugin.zip RtcmPlugin/bin/RtcmPlugin RtcmPlugin/manifest.json
ln -s ../bin SpatPlugin/bin
zip SpatPlugin.zip SpatPlugin/bin/SpatPlugin SpatPlugin/manifest.json
ln -s ../bin PreemptionPlugin/bin
zip PreemptionPlugin.zip PreemptionPlugin/bin/PreemptionPlugin PreemptionPlugin/manifest.json
ln -s ../bin MessageLoggerPlugin/bin
zip MessageLoggerPlugin.zip MessageLoggerPlugin/bin/MessageLoggerPlugin MessageLoggerPlugin/manifest.json
ln -s ../bin PedestrianPlugin/bin
zip PedestrianPlugin.zip PedestrianPlugin/bin/PedestrianPlugin PedestrianPlugin/manifest.json
ln -s ../bin TimPlugin/bin
zip TimPlugin.zip TimPlugin/bin/TimPlugin TimPlugin/manifest.json
ln -s ../bin CARMACloudPlugin/bin
zip CARMACloudPlugin.zip CARMACloudPlugin/bin/CARMACloudPlugin CARMACloudPlugin/manifest.json
ln -s ../bin PortDrayagePlugin/bin
zip PortDrayagePlugin.zip PortDrayagePlugin/bin/PortDrayagePlugin PortDrayagePlugin/manifest.json
ln -s ../bin ODELoggerPlugin/bin
zip ODELoggerPlugin.zip ODELoggerPlugin/bin/ODELoggerPlugin ODELoggerPlugin/manifest.json
ln -s ../bin CARMAStreetsPlugin/bin
zip CARMAStreetsPlugin.zip CARMAStreetsPlugin/bin/CARMAStreetsPlugin CARMAStreetsPlugin/manifest.json
