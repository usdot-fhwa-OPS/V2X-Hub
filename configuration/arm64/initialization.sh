#!/bin/bash

set -e

# Max query attempts before consider setup failed
MAX_TRIES=20

if
  cat initializationLog.txt | grep "First Initialization Complete."
then
  function postInitialization() {
    sudo docker-compose up -d
  }

  postInitialization "MariaDB"

else
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
  # Return true-like values if and only if logs
  # contain the expected "ready" line
  function dbIsReady() {
    sudo docker-compose logs db | grep "MySQL init process done. Ready for start up."
  }

  function waitUntilServiceIsReady() {
    attempt=1
    while [ $attempt -le $MAX_TRIES ]; do
      if "$@"; then
        echo "MariaDB container is up!"
        echo "First Initialization Complete." >> initializationLog.txt
	sleep 60
        sudo rm docker-compose.yml
        sudo mv post-initialization.yml docker-compose.yml
        sudo docker stop $(sudo docker ps -a -q)
        sudo docker-compose up -d
        sleep 5
        break
      fi
      echo "Waiting for MariaDB container... (attempt: $((attempt++)))"
      sleep 60
    done

    if [ $attempt -gt $MAX_TRIES ]; then
      echo "Error: MariaDB not responding, cancelling set up"
      exit 1
    fi
  }

  waitUntilServiceIsReady dbIsReady "MariaDB"

fi
