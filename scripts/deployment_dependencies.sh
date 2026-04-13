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
DEPENDENCIES="ca-certificates \
    libboost-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libboost-log-dev \
    libboost-chrono-dev \
    libboost-atomic-dev \
    libboost-regex-dev \
    libboost-filesystem-dev \
    libboost-program-options-dev \
    libgps-dev \
    libjsoncpp-dev \
    libmysqlclient-dev \
    libmysqlcppconn-dev \
    libperl-dev \
    librdkafka-dev \
    libssl-dev \
    libwebsockets-dev \
    libxerces-c-dev \
    qtbase5-dev \
    uuid-dev \
    zip \
    zlib1g \
    libprotobuf-c-dev \
    curl \
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
