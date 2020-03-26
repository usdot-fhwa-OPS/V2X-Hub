#!/bin/sh -x

USER=`head -1 ./.env | cut -d "=" -f 2`
PASS=`tail -1 ./.env | cut -d "=" -f 2`

mysql -uroot -pivp -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('$USER', '$PASS', 3)"
