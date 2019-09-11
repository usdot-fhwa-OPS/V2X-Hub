#!/bin/bash 
echo "Usage: sh ./run.sh <toplevel V2X-Hub directory>"

if [ $# -lt 1 ]
then
	echo "Error [ Usage: sh ./run.sh <toplevel V2X-Hub directory>] \nExiting"
	exit
fi

echo "Top level dir: $1"
TOPDIR=$1



cd $TOPDIR/src/tmx/TmxCore/
sudo cp tmxcore.service /lib/systemd/system/
sudo cp tmxcore.service /usr/sbin/
cd $TOPDIR/container/
chmod +x database.sh
./database.sh
cd $TOPDIR/container/
chmod +x service.sh

cd  /var/www/
sudo mkdir ~/plugins
cd  $TOPDIR/src/v2i-hub/
sudo tmxctl --plugin-install CommandPlugin.zip
cd /var/www/plugins/
mkdir /var/www/plugins/.ssl
sudo chown plugin .ssl
sudo chgrp www-data .ssl
cd /var/www/plugins/.ssl/
openssl req -x509 -newkey rsa:4096 -sha256 -nodes -keyout tmxcmd.key -out tmxcmd.crt -subj "/CN=192.168.55.46" -days 3650
sudo chown plugin *
sudo chgrp www-data *
cd $TOPDIR/src/v2i-hub/
sudo tmxctl --plugin-install CswPlugin.zip
sudo tmxctl --plugin-install DmsPlugin.zip
sudo tmxctl --plugin-install DsrcImmediateForwardPlugin.zip
sudo tmxctl --plugin-install LocationPlugin.zip
sudo tmxctl --plugin-install MapPlugin.zip
sudo tmxctl --plugin-install MessageReceiverPlugin.zip
sudo tmxctl --plugin-install ODEPlugin.zip
sudo tmxctl --plugin-install RtcmPlugin.zip
sudo tmxctl --plugin-install SpatPlugin.zip
sudo tmxctl --plugin-install PreemptionPlugin.zip
sudo tmxctl --plugin-install SPaTLoggerPlugin.zip
sudo tmxctl --plugin-install BsmLoggerPlugin.zip

$TOPDIR/container/service.sh
