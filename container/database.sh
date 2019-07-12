#!/bin/sh -x

mysql -uroot -pivp -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('v2xadmin', 'V2xHub#321', 3)"

set +e
id plugin >/dev/null 2>&1
if [ $? -ne 0 ]; then
	adduser --system --disabled-login --disabled-password --gecos --no-create-home plugin
fi

usermod -a -G dialout plugin

mkdir -p /var/log/tmx
chmod 755 /var/log/tmx

mkdir -p /var/www/plugins
chown plugin:adm /var/www /var/www/plugins
chmod 755 /var/www/plugins
