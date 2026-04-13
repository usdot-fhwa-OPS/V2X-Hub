#!/bin/sh

# exit on errors
set -ex

# Get ubuntu distribution code name. All STOL APT debian packages are pushed to S3 bucket based on distribution codename.
. /etc/lsb-release

# add the STOL APT repository
echo "deb [trusted=yes] http://s3.amazonaws.com/stol-apt-repository develop ${DISTRIB_CODENAME}" > /etc/apt/sources.list.d/stol-apt-repository.list
apt-get clean
apt-get update --fix-missing

# NOTE: libwebsockets-dev from Ubuntu 20 on is sufficient
# file is required for netsnmp installation under ext/
# ca-certificates is required for git 
DEPENDENCIES="build-essential \
    cmake \
    git \
    ca-certificates \
    file \
    libboost-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libboost-log-dev \
    libboost-chrono-dev \
    libboost-atomic-dev \
    libboost-regex-dev \
    libboost-filesystem-dev \
    libboost-program-options-dev \
    libcurl4-openssl-dev \
    libev-dev \
    libgps-dev \
    libgtest-dev \
    libjsoncpp-dev \
    libmysqlclient-dev \
    libmysqlcppconn-dev \
    libperl-dev \
    librdkafka-dev \
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
    curl \
    gdb \
    mysql-client"

# STOL library dependencies
LIBRARY_DEPENDENCIES=" \
    carma-clock-1 \
    stol-j2735-201603-carma-1 \
    stol-j2735-2020-carma-1 \
    stol-j2735-2024-carma-1 \
"

# install all things needed for deployment, always done
apt-get install -y --no-install-recommends $DEPENDENCIES ${LIBRARY_DEPENDENCIES}

numCPU=$(nproc)

# install gtest
cd /usr/src/googletest/
mkdir -p build/
cd build
cmake ..
make -j${numCPU}
make install
