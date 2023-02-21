#!/bin/sh
set -ex

cp src/tmx/TmxCore/tmxcore.service /lib/systemd/system/
cp src/tmx/TmxCore/tmxcore.service /usr/sbin/

mkdir -p /var/www/plugins/MAP /var/www/plugins/.ssl
openssl req -x509 -newkey rsa:4096 -sha256 -nodes -keyout /var/www/plugins/.ssl/tmxcmd.key -out /var/www/plugins/.ssl/tmxcmd.crt -subj "/CN= <127.0.0.1> " -days 3650
chown -R plugin:www-data /var/www/plugins/.ssl
