#!/bin/bash

# exit on errors
set -e

dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
mainDir="`( cd \"$dir\" && cd ../ && pwd )`"

sed -i 's|http://archive.ubuntu.com|http://us.archive.ubuntu.com|g' /etc/apt/sources.list
$mainDir/scripts/install_dependencies.sh

# build out ext components
cd $mainDir/ext/
./build.sh

cd $mainDir/container/
cp wait-for-it.sh /usr/local/bin/
cp service.sh /usr/local/bin/

./database.sh
./library.sh
ldconfig

# build internal components
cd $mainDir/src/
./build.sh release
ldconfig

$mainDir/scripts/deployment_dependencies.sh

cp $mainDir/src/tmx/TmxCore/tmxcore.service /lib/systemd/system/
cp $mainDir/src/tmx/TmxCore/tmxcore.service /usr/sbin/
ldconfig

$mainDir/container/setup.sh

cd /var/log/tmx/
/usr/local/bin/service.sh
