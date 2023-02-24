#!/bin/bash
sudo apt-get -y remove docker docker-engine docker.io containerd runc
sudo apt-get update
sudo apt-get -y install apt-transport-https ca-certificates curl gnupg-agent software-properties-common
OS=$(lsb_release -i | awk 'FS=":" {print $3;}' | awk '{print tolower($0)}')
arch=$(dpkg --print-architecture)
curl -fsSL https://download.docker.com/linux/$OS/gpg | sudo apt-key add -
sudo add-apt-repository "deb [arch=$arch] https://download.docker.com/linux/$OS $(lsb_release -cs) stable"
sudo apt-get update
sudo apt-get -y install docker-ce docker-ce-cli containerd.io docker-compose-plugin
sudo apt -y install python3-pip
sudo pip3 install docker-compose
sudo docker-compose up -d
