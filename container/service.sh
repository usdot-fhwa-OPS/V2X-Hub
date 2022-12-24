#!/bin/bash
chmod +x /home/V2X-Hub/container/wait-for-it.sh
/home/V2X-Hub/container/wait-for-it.sh 127.0.0.1:3306
chmod 777 /var/log/tmx
echo ${MYSQL_ROOT_PASSWORD}
tmxctl --plugin-install CommandPlugin.zip
tmxctl --plugin-install CswPlugin.zip
tmxctl --plugin-install DmsPlugin.zip
tmxctl --plugin-install DsrcImmediateForwardPlugin.zip
tmxctl --plugin-install LocationPlugin.zip
tmxctl --plugin-install MapPlugin.zip
tmxctl --plugin-install MessageReceiverPlugin.zip
tmxctl --plugin-install RtcmPlugin.zip
tmxctl --plugin-install SpatPlugin.zip
tmxctl --plugin-install PreemptionPlugin.zip
tmxctl --plugin-install MessageLoggerPlugin.zip
tmxctl --plugin-install PedestrianPlugin.zip
tmxctl --plugin-install TimPlugin.zip
tmxctl --plugin-install CARMACloudPlugin.zip
tmxctl --plugin-install CARMAStreetsPlugin.zip
tmxctl --plugin-install ODELoggerPlugin.zip
tmxctl --plugin-install PortDrayagePlugin.zip
tmxctl --plugin-install ERVCloudForwardingPlugin.zip

tmxctl --plugin CommandPlugin --enable
/usr/local/bin/tmxcore
