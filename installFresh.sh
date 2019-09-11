#!/bin/bash 
echo "Usage: sh ./installFresh.sh <toplevel V2X-Hub directory>"

if [ $# -lt 1 ]
then
	echo "Error [ Usage: sh ./installFresh.sh <toplevel V2X-Hub directory>] \nExiting"
	exit
fi

echo "Top level dir: $1"
TOPDIR=$1

cd $TOPDIR/src/tmx
make clean
cmake .
make 
sudo make install 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib


cd $TOPDIR/ext/libwebsockets
cmake -DLWS_WITH_SHARED=OFF .
make
sudo make install

cd $TOPDIR/src/v2i-hub/ 
make clean
cmake .
make
