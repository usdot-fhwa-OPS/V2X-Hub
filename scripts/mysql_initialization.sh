#!/bin/sh

# exit on errors
# set -e

directory="`dirname \"$0\"`"
mainDir="`( cd \"$directory\" && cd ../ && pwd )`"

# Ensure mysql-server is installed
REQUIRED_PKG="mysql-server"
PKG_OK="`( dpkg-query -W --showformat='${Status}\n' $REQUIRED_PKG )`"
echo Checking for $REQUIRED_PKG: $PKG_OK
if [ "" = "$PKG_OK" ]; then
  echo "No $REQUIRED_PKG found. Installing $REQUIRED_PKG."
  apt-get update
  apt-get -y install $REQUIRED_PKG
fi

# Make passwords for mysql
mkdir -p /run/secrets
secretsDir=/run/secrets
cd $secretsDir

FILE1=mysql_root_password.txt
FILE2=mysql_password.txt
if test -f "$FILE1"; then
    echo "$FILE1 exists."
else
    read -p "enter password for the mysql_root_password: " sql_root_pass
    echo "$sql_root_pass" > sql_root_pass.txt
    #remove endline characters from password files
    tr -d '\n' <sql_root_pass.txt> mysql_root_password && rm sql_root_pass.txt
fi

if test -f "$FILE2"; then
    echo "$FILE2 exists."
else
    read -p "enter password for mysql_password: " sql_pass
    echo "$sql_pass" > sql_pass.txt
    #remove endline characters from password files
    tr -d '\n' <sql_pass.txt> mysql_password && rm sql_pass.txt

fi

# Set environment values
echo "# V2X HUB ADDITIONS" | tee -a $HOME/.bashrc > /dev/null
echo "export MYSQL_DATABASE=IVP" | tee -a $HOME/.bashrc > /dev/null
echo "export MYSQL_USER=IVP" | tee -a $HOME/.bashrc > /dev/null
echo "export MYSQL_PASSWORD_FILE=/run/secrets/mysql_password" | tee -a $HOME/.bashrc > /dev/null
echo "export MYSQL_ROOT_PASSWORD_FILE=/run/secrets/mysql_root_password" | tee -a $HOME/.bashrc > /dev/null
echo "export MYSQL_PASSWORD=/run/secrets/mysql_password" | tee -a $HOME/.bashrc > /dev/null

# Database setup
MYSQL_ROOT_USER="root"
MYSQL_ROOT_PASSWORD=$(grep -v '^#' $secretsDir/mysql_root_password | xargs -d '\n')
mysql -u$MYSQL_ROOT_USER -h127.0.0.1 -e "ALTER USER '$MYSQL_ROOT_USER'@'localhost' IDENTIFIED BY '$MYSQL_ROOT_PASSWORD';"

# Test out the connection to the server
mysql -u$MYSQL_ROOT_USER -p$MYSQL_ROOT_PASSWORD -h127.0.0.1 -e "SHOW STATUS WHERE Variable_name = 'Uptime' and Value > 0;"

# Install the database
MYSQL_USER="IVP"
MYSQL_PASSWORD=$(grep -v '^#' $secretsDir/mysql_password | xargs -d '\n')

mysql -u$MYSQL_ROOT_USER -p$MYSQL_ROOT_PASSWORD -h127.0.0.1 -e "CREATE USER '$MYSQL_USER'@'localhost' IDENTIFIED BY '$MYSQL_PASSWORD';"
mysql -u$MYSQL_ROOT_USER -p$MYSQL_ROOT_PASSWORD -h127.0.0.1 -e "CREATE DATABASE IF NOT EXISTS $MYSQL_USER;"
mysql -u$MYSQL_ROOT_USER -p$MYSQL_ROOT_PASSWORD -h127.0.0.1 -e "GRANT ALL PRIVILEGES ON $MYSQL_USER.* To '$MYSQL_USER'@'localhost';"

if [ -f $mainDir/configuration/amd64/mysql/localhost.sql ]; then
	mysql -v -u$MYSQL_USER -p --silent < $mainDir/configuration/amd64/mysql/localhost.sql
fi
