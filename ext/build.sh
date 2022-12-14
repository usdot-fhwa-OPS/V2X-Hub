#!/bin/bash

# exit on errors
set -e

# The V2X Hub supplied plugins have a dependency on a version of libwebsockets that is newer than the installable package that comes with Ubuntu. Hence a custom version of the software has benn forked and made available with V2X-Hub.
pushd /tmp
#git clone https://github.com/usdot-fhwa-OPS/libwebsockets.git
wget https://github.com/warmcat/libwebsockets/archive/refs/tags/v4.2.0.tar.gz
tar xvf v4.2.0.tar.gz
cd libwebsockets-4.2.0/
cmake -DLWS_WITH_SHARED=OFF -DLWS_SUPPRESS_DEPRECATED_API_WARNINGS=ON .
make
make install
popd

# An OPENAPI based Qt webservice is needed by the plugins for http requests processing. A custom generated code using OPENAPI framework is located in GitHub.
pushd /tmp
git clone https://github.com/usdot-fhwa-OPS/qhttpengine.git
cd qhttpengine/
cmake .
make 
make install
popd

pushd /tmp
git clone https://github.com/HowardHinnant/date.git
cd date
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
