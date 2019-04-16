#!/bin/sh -x


DBROOTUSER="root"

DBROOTPASS=$MYSQL_ROOT_PASSWORD

# Test out the connection to the server
mysql -u$DBROOTUSER -p$DBROOTPASS --silent -e "SHOW STATUS WHERE Variable_name = 'Uptime' and Value > 0;"

# Install the database
DBUSER="IVP"
DBPASS=$MYSQL_ROOT_PASSWORD
mysql -u$DBROOTUSER -p$DBROOTPASS -e "CREATE DATABASE IF NOT EXISTS $DBUSER; GRANT ALL PRIVILEGES ON $DBUSER.* To '$DBUSER'@'localhost' IDENTIFIED BY '$DBPASS';"
if [ -f ./localhost.sql ]; then
	mysql -v -u$DBUSER -p$DBPASS < ./localhost.sql
fi

mysql -u$DBROOTUSER -p$DBROOTPASS -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('_battelle', 'B@ttelle', 3)"

mysql -u$DBROOTUSER -p$DBROOTPASS -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('v2iadmin', 'V2iHub#321', 3)"

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
