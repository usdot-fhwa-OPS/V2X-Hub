#!/bin/sh -x

mysql -uroot -pivp --silent -e "SHOW STATUS WHERE Variable_name = 'Uptime' and Value > 0;"
mysql -uroot -pivp -e "CREATE DATABASE IF NOT EXISTS IVP; GRANT ALL PRIVILEGES ON IVP.* To 'IVP'@'127.0.0.1' IDENTIFIED BY 'ivp';"
mysql -uroot -pivp -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('v2xadmin', 'V2xHub#321', 3)"
mysql -v -uIVP -pivp IVP < /docker-entrypoint-initdb.d/localhost.sql
