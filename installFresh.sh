#!/bin/bash 
echo "Usage: sh ./installFresh.sh <toplevel V2X-Hub directory>"

if [ $# -lt 1 ]
then
	echo "Error [ Usage: sh ./installFresh.sh <toplevel V2X-Hub directory>] \nExiting"
	exit
fi

echo "Top level dir: $1"
TOPDIR=$1

### Setup tmxcore 

cd $TOPDIR/src/tmx
make clean
cmake .
make 
sudo make install 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

## work with libwebsockets

mkdir -p $TOPDIR/ext

cd $TOPDIR/ext/
git clone https://github.com/usdot-fhwa-OPS/libwebsockets.git
cd $TOPDIR/ext/libwebsockets
cmake -DLWS_WITH_SHARED=OFF .
make
sudo make install

### make server shared object for Pedestrian Plugin 

cd $TOPDIR/ext/server
cmake .
make 
sudo make install 


### setup and install v2x-hub core and plugins 

cd $TOPDIR/src/v2i-hub/ 
make clean
cmake . -DqserverPedestrian_DIR=/usr/local/share/qserverPedestrian/cmake
make

### create plugins for installation 

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
  ln -s ../bin PedestrianPlugin/bin
  zip PedestrianPlugin.zip PedestrianPlugin/bin/PedestrianPlugin PedestrianPlugin/manifest.json

