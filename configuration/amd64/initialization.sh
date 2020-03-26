#!/bin/bash

# takes user-provided username and password for docker-compose.yml and stores in .env file
echo "Please enter a username: "
read USER
echo "Password must be 8-12 charcters, and contain at least one of each of the following: uppercase letter, lowercase letter, number, and symbol."
echo "Please enter a password: "
read -s PASS
  
PASS_LENGTH=`echo $PASS | wc -c`

if [ $PASS_LENGTH -ge 8 ] && echo $PASS | grep -q [a-z] && echo $PASS | grep -q [A-Z] && echo $PASS | grep -q [0-9] && echo $PASS | grep -q [\$\!\.\+_-\*@\#\^%\?~]; then
    sudo echo "username=$USER" > .env
    sudo echo "password=$PASS" >> .env
    sudo echo "VALID PASSWORD"
else
    sudo echo "INVALID PASSWORD"
    sudo echo "Password must be 8-12 charcters, and contain at least one of each of the following: uppercase letter, lowercase letter, number, and symbol"
    exit 1
fi

sudo apt-get -y remove docker docker-engine docker.io containerd runc
sudo apt-get update
sudo apt-get -y install apt-transport-https ca-certificates curl gnupg-agent software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
sudo apt-get update
sudo apt-get -y install docker-ce docker-ce-cli containerd.io 
sudo apt -y install python3-pip
sudo pip3 install docker-compose
sudo docker-compose up -d