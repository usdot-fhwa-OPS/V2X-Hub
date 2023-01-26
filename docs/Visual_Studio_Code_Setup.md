# Using VS Code For Development

[Visual Studio Code](https://code.visualstudio.com/) is a functional free IDE which can be used to develop many projects. Through the use of Docker images it allows developers to develop in containers where build tools can be preconfigured, precluding the need to have a special host to build on with instructions to follow for tool installation.  It can be used on Windows hosts using Docker Desktop or on Linux hosts with Docker installed.

## General Setup

See https://code.visualstudio.com/docs/devcontainers/containers as a reference on setting up your environment.  Here is a summary of the steps:
* Install Docker
* Install VS Code
* Open the source folder on your host.  VS Code should notify you that there are extensions available to handle the .devcontainer folder. Open Folder in Container

## Other setup

* Secrets files

If you want to reset the database:
docker stop mysql
docker rm mysql
docker volume rm v2x-hub_devcontainer_mysql-datavolume
Then rebuild container
