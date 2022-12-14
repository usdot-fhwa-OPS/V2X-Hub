#!/bin/sh

# exit on errors
set -e

apt-get update

# install all things needed for deployment, always done
apt-get install -y \
    cmake \
    git \
    build-essential \
    libgtest-dev \
    libssl-dev \
    qtbase5-dev \
    zip \
    wget \
    libmysqlcppconn-dev \
    libboost-all-dev \
    libmysqlclient-dev \
    uuid-dev \
    zlib1g \
    libxerces-c-dev \
    libcurl4-openssl-dev \
    libgps-dev \
    libsnmp-dev \
    librdkafka-dev \
    libjsoncpp-dev \
    libev-dev \
    libuv1-dev \
    libcap-dev \
    libcpprest-dev
