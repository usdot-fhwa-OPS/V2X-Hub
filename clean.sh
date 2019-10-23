#!/bin/bash 
echo "Usage: sh ./clean.sh <toplevel V2X-Hub directory>"

if [ $# -lt 1 ]
then
        echo "Error [ Usage: sh ./clean.sh <toplevel V2X-Hub directory>] \nExiting"
        exit
fi

echo "Top level dir: $1"
TOPDIR=$1



cd $TOPDIR/src/tmx
make clean

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

cd $TOPDIR/src/v2i-hub/
make clean
