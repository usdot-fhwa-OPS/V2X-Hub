#!/bin/bash
# Fail when any command fails
#set -e

# Check if V2XHUB_VERSION is passed
if [ -n "$1" ]; then
    V2XHUB_VERSION="$1"
else
    # Prompt user for V2XHUB_VERSION
    read -r -p "Enter the deployed V2X-Hub version number: " V2XHUB_VERSION
fi

echo "Adding V2X-Hub user for version: $V2XHUB_VERSION"

# Ensure mysql-client is installed
arch=$(dpkg --print-architecture)
# TODO: Add a common mysql-client that works for ARM and AMD devices 
if [ $arch = "amd64" ]; then
    REQUIRED_PKG="mysql-client"
else
    REQUIRED_PKG="mariadb-client"
fi
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
    # Check if V2XHUB_VERSION is >= 7.5.0
    if [[ "$(echo "$V2XHUB_VERSION 7.5.0" | awk '{print ($1 >= $2)}')" -eq 1 ]]; then
        mysql -uroot -p --silent -h127.0.0.1 -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('$USER', SHA2('$PASS', 256), 3)"
    elif [[ "$(echo "$V2XHUB_VERSION develop" | awk '{print ($1 == $2)}')" -eq 1 ]]; then
        mysql -uroot -p --silent -h127.0.0.1 -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('$USER', SHA2('$PASS', 256), 3)"
    else
        mysql -uroot -p --silent -h127.0.0.1 -e "INSERT INTO IVP.user (IVP.user.username, IVP.user.password, IVP.user.accessLevel) VALUES('$USER', '$PASS', 3)"
    fi
    echo "V2X Hub user successfully added"
else
    echo "INVALID PASSWORD"
    echo "Password must be 8-12 charcters, and contain at least one of each of the following: uppercase letter, lowercase letter, number, and symbol"
    exit 1
fi
