#!/bin/sh -x
mysql -uroot -pivp --silent -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('$username', '$password', 3)"
