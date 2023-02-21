#!/bin/sh

# exit on errors
set -e

apt-get update

# NOTE: libwebsockets-dev from Ubuntu 20 on is sufficient
DEPENDENCIES="build-essential \
    cmake \
    git \
    libboost-all-dev \
    libcpprest-dev \
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
    curl"

# install all things needed for deployment, always done
apt-get install -y $DEPENDENCIES
