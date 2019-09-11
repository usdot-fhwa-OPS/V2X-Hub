#!/bin/bash 
echo "Usage: sh ./installtmx.sh <toplevel V2X-Hub directory>"

if [ $# -lt 1 ]
then
        echo "Error [ Usage: sh ./installtmx.sh <toplevel V2X-Hub directory>] \nExiting"
        exit
fi

echo "Top level dir: $1"
TOPDIR=$1



cd $TOPDIR/src/tmx
cmake .
make 
sudo make install 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

cd $TOPDIR
mkdir -p ext && cd ext
git clone git@github.com:usdot-fhwa-OPS/libwebsockets.git
cd libwebsockets
cmake -DLWS_WITH_SHARED=OFF .
make
sudo make install


