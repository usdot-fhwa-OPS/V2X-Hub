#!/bin/sh -x


DBROOTUSER="root"

DBROOTPASS="ivp"

# Test out the connection to the server
mysql -u$DBROOTUSER -p$DBROOTPASS --silent -e "SHOW STATUS WHERE Variable_name = 'Uptime' and Value > 0;"

# Install the database
DBUSER="IVP"
DBPASS="ivp"
mysql -u$DBROOTUSER -p$DBROOTPASS -e "CREATE DATABASE IF NOT EXISTS $DBUSER; GRANT ALL PRIVILEGES ON $DBUSER.* To '$DBUSER'@'localhost' IDENTIFIED BY '$DBPASS';"
if [ -f ./localhost.sql ]; then
	mysql -v -u$DBUSER -p$DBPASS < ./localhost.sql
fi

mysql -u$DBROOTUSER -p$DBROOTPASS -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('_battelle', 'B@ttelle', 3)"

mysql -u$DBROOTUSER -p$DBROOTPASS -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('v2iadmin', 'V2iHub#321', 3)"
