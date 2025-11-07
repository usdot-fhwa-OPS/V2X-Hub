#!/bin/sh -x
set +e
id plugin >/dev/null 2>&1
if [ $? -ne 0 ]; then
	adduser --system --disabled-login --disabled-password --gecos --no-create-home plugin
fi

usermod -a -G dialout,adm,www-data plugin

mkdir -p /var/www/download /var/www/plugins/ssl /var/log/tmx
chown plugin:adm -R /var/www/ /var/log/tmx
chmod 755 -R /var/www/
