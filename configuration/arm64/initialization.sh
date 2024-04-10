#!/bin/bash
directory=$(pwd)
mysqlDir="$directory/mysql"

# update and upgrade commands to update linux OS
sudo apt update -y && sudo apt upgrade -y

#installing necessary and useful apps
sudo apt-get install chromium-browser -y      #Chrome required for CARMA platform/V2X Hub UI(?)
sudo apt install curl -y                      #Curl for downloading files over internet

#install docker
curl -L https://raw.githubusercontent.com/usdot-fhwa-stol/carma-platform/develop/engineering_tools/install-docker.sh | bash 

#make passwords for mysql
mkdir -p secrets && cd secrets

#creates password files where user inputs password
FILE1=mysql_root_password.txt
FILE2=mysql_password.txt
if test -f "$FILE1"; then
    echo "$FILE1 exists."
else
    read -p "enter password for the mysql_root_password: " sql_root_pass
    echo "$sql_root_pass" > sql_root_pass.txt
    #remove endline characters from password files
    tr -d '\n' <sql_root_pass.txt> mysql_root_password.txt && rm sql_root_pass.txt
fi

if test -f "$FILE2"; then
    echo "$FILE2 exists."
else
    read -p "enter password for mysql_password: " sql_pass
    echo "$sql_pass" > sql_pass.txt
    #remove endline characters from password files
    tr -d '\n' <sql_pass.txt> mysql_password.txt && rm sql_pass.txt
fi

#ARM initialization
cd $directory
sudo apt-get -y remove docker docker-engine docker.io containerd runc
sudo apt-get -y install apt-transport-https ca-certificates curl gnupg-agent software-properties-common
OS=$(lsb_release -i | awk 'FS=":" {print $3;}' | awk '{print tolower($0)}')
arch=$(dpkg --print-architecture)
curl -fsSL https://download.docker.com/linux/$OS/gpg | sudo apt-key add -
sudo add-apt-repository "deb [arch=$arch] https://download.docker.com/linux/$OS $(lsb_release -cs) stable"
sudo apt-get -y install docker-ce docker-ce-cli containerd.io docker-compose-plugin
sudo apt -y install python3-pip
sudo pip3 install docker-compose
sudo apt update -y && sudo apt upgrade -y
sudo docker-compose up -d

#create v2xhub user
cd $mysqlDir
./add_v2xhub_user.bash

chromium-browser "http://127.0.0.1" > /dev/null 2>&1 &
chromium-browser "https://127.0.0.1:19760" > /dev/null 2>&1 &
