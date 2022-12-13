#!/bin/sh

# exit on errors
set -e

DEPLOY_IMAGE=0
if [ "$1" = "deploy" ]; then
    DEPLOY_IMAGE=1
fi

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
    libmysqlcppconn-dev \
    libboost1.71-all-dev \
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
    libcpprest-dev

if [ $DEPLOY_IMAGE -eq 0 ]; then
# install other things needed for development
apt-get install -y \
    sudo \
    libusb-dev \
    libusb-1.0-0-dev \
    libftdi-dev \
    swig \
    liboctave-dev \
    gpsd \
    portaudio19-dev \
    libsndfile1-dev \
    libglib2.0-dev \
    libglibmm-2.4-dev \
    libpcre3-dev \
    libsigc++-2.0-dev \
    libxml++2.6-dev \
    libxml2-dev \
    liblzma-dev \
    dpkg-dev \
    vim \
    qtbase5-dev-tools \
    curl \
    libqhttpengine-dev
fi


