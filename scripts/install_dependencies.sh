#!/bin/sh

# exit on errors
set -e

apt-get update

DEPENDENCIES="build-essential \
    cmake \
    git \
    libboost1.71-all-dev \
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
    libxerces-c-dev \
    qtbase5-dev \
    uuid-dev \
    zip \
    zlib1g"

# install all things needed for deployment, always done
apt-get install -y $DEPENDENCIES
