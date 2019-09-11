#!/bin/bash 
echo "Usage: sh ./installv2i.sh <toplevel V2X-Hub directory>"

if [ $# -lt 1 ]
then
        echo "Error [ Usage: sh ./installv2i.sh <toplevel V2X-Hub directory>] \nExiting"
        exit
fi

echo "Top level dir: $1"
TOPDIR=$1



cd $TOPDIR/src/v2i-hub/ 
cmake .
make
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
 ln -s ../bin ODEPlugin/bin
 zip ODEPlugin.zip ODEPlugin/bin/ODEPlugin ODEPlugin/manifest.json
 ln -s ../bin RtcmPlugin/bin
 zip RtcmPlugin.zip RtcmPlugin/bin/RtcmPlugin RtcmPlugin/manifest.json
 ln -s ../bin SpatPlugin/bin
 zip SpatPlugin.zip SpatPlugin/bin/SpatPlugin SpatPlugin/manifest.json
 ln -s ../bin PreemptionPlugin/bin
 zip PreemptionPlugin.zip PreemptionPlugin/bin/PreemptionPlugin PreemptionPlugin/manifest.json
 ln -s ../bin SPaTLoggerPlugin/bin
 zip SPaTLoggerPlugin.zip SPaTLoggerPlugin/bin/SPaTLoggerPlugin SPaTLoggerPlugin/manifest.json
 ln -s ../bin BsmLoggerPlugin/bin
 zip BsmLoggerPlugin.zip BsmLoggerPlugin/bin/BsmLoggerPlugin BsmLoggerPlugin/manifest.json


