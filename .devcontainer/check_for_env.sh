#!/bin/sh
if [ ! -f ./configuration/.env ]; then
    echo ": .env file not found in /home/V2X-Hub/configuration/. Copying devcontainer .env to configuration directory."
    cp .devcontainer/devcontainer.env ./configuration/.env
fi