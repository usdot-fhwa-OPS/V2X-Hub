#!/bin/sh

# exit on errors
set -e

# add the STOL APT repository
echo "deb [trusted=yes] http://s3.amazonaws.com/stol-apt-repository develop main" > /etc/apt/sources.list.d/stol-apt-repository.list

apt-get update

# NOTE: libwebsockets-dev from Ubuntu 20 on is sufficient
DEPENDENCIES="build-essential \
    cmake \
    git \
    libboost-all-dev \
    libcurl4-openssl-dev \
    libev-dev \
    libgps-dev \
    libgtest-dev \
    libjsoncpp-dev \
    libmysqlclient-dev \
    libmysqlcppconn-dev \
    librdkafka-dev \
    libsnmp-dev \
    libssl-dev \
    libuv1-dev \
    libwebsockets-dev \
    libxerces-c-dev \
    qtbase5-dev \
    uuid-dev \
    wget \
    zip \
    zlib1g \
    rapidjson-dev \
    librapidxml-dev \
    libprotobuf-c-dev \
    curl"

# STOL library dependencies
LIBRARY_DEPENDENCIES=" \
    carma-clock-1 \
    libpqxx-dev \
"

# install all things needed for deployment, always done
apt-get install -y $DEPENDENCIES ${LIBRARY_DEPENDENCIES}

numCPU=$(nproc)

# install gtest
cd /usr/src/googletest/
mkdir -p build/
cd build
cmake ..
make -j${numCPU}
make install
