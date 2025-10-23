#!/bin/sh -x
set +e
id plugin >/dev/null 2>&1
if [ $? -ne 0 ]; then
	adduser --system --disabled-login --disabled-password --gecos --no-create-home plugin
fi

usermod -a -G dialout plugin

mkdir -p /var/log/tmx
chown plugin:adm /var/log/tmx
chmod 777 /var/log/tmx

mkdir -p /var/www/plugins
chown plugin:adm -R /var/www /var/www/plugins
chmod 755 /var/www/plugins
