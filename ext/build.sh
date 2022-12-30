#!/bin/bash

# exit on errors
set -e

# An OPENAPI based Qt webservice is needed by the plugins for http requests processing. A custom generated code using OPENAPI framework is located in GitHub.
pushd /tmp
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

# GPS Parser
pushd /tmp
git clone https://github.com/ckgt/NemaTode.git
cd NemaTode
cmake .
make
make install
popd

# SNMP library
pushd /tmp
wget http://sourceforge.net/projects/net-snmp/files/net-snmp/5.9.1/net-snmp-5.9.1.tar.gz
tar -xvzf net-snmp-5.9.1.tar.gz
cd net-snmp-5.9.1
./configure --with-default-snmp-version="3" --with-sys-contact="@@no.where" --with-sys-location="Unknown" --with-logfile="/var/log/snmpd.log" --with-persistent-directory="/var/net-snmp"
make
make install
popd