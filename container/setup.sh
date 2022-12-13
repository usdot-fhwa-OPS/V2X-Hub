#!/bin/sh -x
set -e

cd /home/V2X-Hub/src/tmx/TmxCore/
cp tmxcore.service /lib/systemd/system/ && cp tmxcore.service /usr/sbin/
cd /home/V2X-Hub/container/
chmod +x /home/V2X-Hub/container/service.sh && chmod +x /home/V2X-Hub/container/wait-for-it.sh
cd /var/www/
mkdir ~/plugins
cd /var/www/plugins/
mkdir /var/www/plugins/MAP
mkdir /var/www/plugins/.ssl
chown plugin .ssl
chgrp www-data .ssl
cd /var/www/plugins/.ssl/
openssl req -x509 -newkey rsa:4096 -sha256 -nodes -keyout tmxcmd.key -out tmxcmd.crt -subj "/CN= <127.0.0.1> " -days 3650
chown plugin *
chgrp www-data *
