#!/bin/bash

# exit on errors
set -e

# An OPENAPI based Qt webservice is needed by the plugins for http requests processing. A custom generated code using OPENAPI framework is located in GitHub.
pushd /tmp
#git clone https://github.com/usdot-fhwa-OPS/qhttpengine.git
QHTTPENGINE_VERSION=1.0.1
wget -O qhttpengine-${QHTTPENGINE_VERSION}.tar.gz https://github.com/nitroshare/qhttpengine/archive/refs/tags/${QHTTPENGINE_VERSION}.tar.gz
tar xvf qhttpengine-${QHTTPENGINE_VERSION}.tar.gz
cd qhttpengine-${QHTTPENGINE_VERSION}/
cmake .
make 
make install
popd

pushd /tmp
DATELIB_VERSION=3.0.1
wget -O date-${DATELIB_VERSION}.tar.gz https://github.com/HowardHinnant/date/archive/refs/tags/v${DATELIB_VERSION}.tar.gz
tar xvf date-${DATELIB_VERSION}.tar.gz
cd date-${DATELIB_VERSION}/
cmake .
make
make install
popd

ldconfig 

# Server for the Qt webservice
pushd server
cmake .
make
make install
popd

pushd ccserver
cmake . 
make
make install
popd

pushd pdclient
cmake .
make
make install
popd
