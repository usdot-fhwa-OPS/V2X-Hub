#!/bin/sh -x

mysql -uroot --silent -e "SHOW STATUS WHERE Variable_name = 'Uptime' and Value > 0;"
mysql -uroot -e "CREATE DATABASE IF NOT EXISTS IVP; GRANT ALL PRIVILEGES ON IVP.* To 'IVP'@'127.0.0.1' IDENTIFIED BY 'ivp';"
mysql -v -uIVP -pivp -h127.0.0.1 IVP < ./localhost.sql
mysql -uroot -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('v2xadmin', 'V2xHub#321', 3)"
