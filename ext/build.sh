#!/bin/bash

# exit on errors
set -e

# Find number of cores available
numCPU=$(nproc)

# An OPENAPI based Qt webservice is needed by the plugins for http requests processing. A custom generated code using OPENAPI framework is located in GitHub.
pushd /tmp
QHTTPENGINE_VERSION=1.0.1
wget -O qhttpengine-${QHTTPENGINE_VERSION}.tar.gz https://github.com/nitroshare/qhttpengine/archive/refs/tags/${QHTTPENGINE_VERSION}.tar.gz
tar xvf qhttpengine-${QHTTPENGINE_VERSION}.tar.gz
cd qhttpengine-${QHTTPENGINE_VERSION}/
cmake .
make -j${numCPU}
make install
popd

pushd /tmp
DATELIB_VERSION=3.0.1
wget -O date-${DATELIB_VERSION}.tar.gz https://github.com/HowardHinnant/date/archive/refs/tags/v${DATELIB_VERSION}.tar.gz
tar xvf date-${DATELIB_VERSION}.tar.gz
cd date-${DATELIB_VERSION}/
cmake .
make -j${numCPU}
make install
popd

ldconfig 

# Server for the Qt webservice
pushd server
cmake .
make -j${numCPU}
make install
popd

pushd ccserver
cmake . 
make -j${numCPU}
make install
popd

pushd pdclient
cmake .
make -j${numCPU}
make install
popd

# GPS Parser
pushd /tmp
if [ -d "NemaTode" ]; then
    rm -r NemaTode
fi
git clone https://github.com/ckgt/NemaTode.git
cd NemaTode
cmake .
make -j${numCPU}
make install
popd

# Nats C API
pushd /tmp
if [ -d "nats.c" ]; then
    rm -r nats.c
fi
git clone https://github.com/nats-io/nats.c --branch v3.7.0
cd nats.c
cmake . -DNATS_BUILD_NO_SPIN=ON
make -j${numCPU}
make install
popd

# pugixml
pushd /tmp
git clone https://github.com/zeux/pugixml.git --branch v1.14
cd pugixml
cmake .
make -j${numCPU}
make install
popd

# net-snmp
pushd /tmp
git clone https://github.com/net-snmp/net-snmp.git --branch v5.9.4
cd net-snmp
./configure --enable-blumenthal-aes --with-default-snmp-version="3" --with-sys-contact="@@no.where" --with-sys-location="Unknown" --with-logfile="/var/log/snmpd.log" --with-persistent-directory="/var/net-snmp"
make -j${numCPU}
make install
echo 'export LD_LIBRARY_PATH="/usr/local/lib/"' >> ~/.bashrc
popd
