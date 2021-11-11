#!/bin/sh
# Fail when any command fails
#set -e
# Ensure mysql-client is installed
#sudo apt-get update
#sudo apt-get install mysql-client

# Adds V2X-Hub user to mysql db
read -p "Please enter a username: " USER
echo "Password must be 8-12 charcters, and contain at least one of each of the following: uppercase letter, lowercase letter, number, and symbol."
read -s -p "Please enter a password: "  PASS

PASS_LENGTH=`echo $PASS | wc -c`

if [ $PASS_LENGTH -ge 8 ] && echo $PASS | grep -q [a-z] && echo $PASS | grep -q [A-Z] && echo $PASS | grep -q [0-9] && echo $PASS | grep -q [\$\!\.\+_-\*@\#\^%\?~]; then
    echo "Confirm password: "
    read -s CONF_PASS
    while [ $CONF_PASS != $PASS ]; do
        echo "Passwords do not match. Please re-enter password: "
        read -s CONF_PASS
    done
    echo "VALID PASSWORD"
    echo "Enter MYSQL ROOT PASSWORD: "
    mysql -uroot -p --silent -h127.0.0.1 -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('$USER', '$PASS', 3)"
else
    echo "INVALID PASSWORD"
    echo "Password must be 8-12 charcters, and contain at least one of each of the following: uppercase letter, lowercase letter, number, and symbol"
    exit 1
fi
