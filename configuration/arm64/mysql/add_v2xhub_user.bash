#!/bin/bash
# Fail when any command fails
#set -e

# Ensure mysql-client is installed
REQUIRED_PKG="mariadb-client-10.5"
PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $REQUIRED_PKG|grep "install ok installed")
echo Checking for $REQUIRED_PKG: $PKG_OK
if [ "" = "$PKG_OK" ]; then
  echo "No $REQUIRED_PKG found. Installing $REQUIRED_PKG."
  sudo apt-get update
  sudo apt-get --yes install $REQUIRED_PKG
fi

# Adds V2X-Hub user to mysql db
read -p "Please enter a username: " USER
echo "Password must be 8-12 charcters, and contain at least one of each of the following: uppercase letter, lowercase letter, number, and symbol."
read -s -p "Please enter a password: "  PASS

PASS_LENGTH=`echo $PASS | wc -c`

if [ $PASS_LENGTH -ge 8 ] && echo $PASS | grep -q [a-z] && echo $PASS | grep -q [A-Z] && echo $PASS | grep -q [0-9] && ( echo $PASS | grep -q [\$\!\.\+_\*@\#\^%\?~] || echo $PASS | grep -q [-] ); then
    echo
    read -s -p "Confirm password: " CONF_PASS
    while [ $CONF_PASS != $PASS ]; do
        echo
        read -s -p "Passwords do not match. Please re-enter password: " CONF_PASS
    done
    echo "VALID PASSWORD"
    echo "Enter MYSQL ROOT PASSWORD: "
    mysql -uroot -p --silent -h127.0.0.1 -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('$USER', SHA2('$PASS', 256), 3)"
    echo "V2X Hub user successfully added"
else
    echo "INVALID PASSWORD"
    echo "Password must be 8-12 charcters, and contain at least one of each of the following: uppercase letter, lowercase letter, number, and symbol"
    exit 1
fi
