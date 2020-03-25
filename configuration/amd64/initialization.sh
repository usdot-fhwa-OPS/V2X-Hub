#!/bin/bash

# takes user-provided username and password for docker-compose.yml and stores in .env file
USERNAME=$1
PASSWORD=$2

if [[ -z $PASSWORD ]]; then
  echo "No password entered."
else
  PASS_LENGTH=`echo $1 | wc -c`

  if [ $PASS_LENGTH -ge 8 ] && echo $1 | grep [a-z] && echo $1 | grep [A-Z] && echo $1 | grep [0-9] && echo $1 | grep [\$\!\.\+_-\*@\#\^%\?~]; then
      sudo echo "username=$1" > .env
      sudo echo "password=$2" >> .env
      sudo echo "VALID PASSWORD"
  else
      sudo echo "INVALID PASSWORD"
      sudo echo "Password must be 8-12 charcters, and contain at least one of each of the following: uppercase letter, lowercase letter, number, and symbol"
      sudo exit 1
  fi
fi

sudo apt-get -y remove docker docker-engine docker.io containerd runc
sudo apt-get update
sudo apt-get -y install apt-transport-https ca-certificates curl gnupg-agent software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
sudo add-apt-repository "deb [arch=arm64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
sudo apt-get update
sudo apt-get -y install docker-ce docker-ce-cli containerd.io 
sudo apt -y install python3-pip
sudo pip3 install docker-compose
sudo docker-compose up -d