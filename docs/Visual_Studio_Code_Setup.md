# Using VS Code For Development

[Visual Studio Code](https://code.visualstudio.com/) is a functional free IDE which can be used to develop many projects. Through the use of Docker images it allows developers to develop in containers where build tools can be pre-configured, precluding the need to have a special host to build on with instructions to follow for tool installation.  It can be used on Windows hosts using Docker Desktop or on Linux hosts with Docker installed.

## Setup

See https://code.visualstudio.com/docs/devcontainers/containers as a reference on setting up your environment.  Here is a summary of the steps below with the assumption that a Linux environment is being used.

### Installation

You will need to have the following installed on your host or VM.

* Docker - Install varies by Linux distribution. Use Docker Desktop on Windows.
* Install VS Code - Look in https://code.visualstudio.com/download

### One Time Setup

* Open the source folder on your host.  VS Code may notify you that there are extensions available to handle the .devcontainer folder.  If so do not do anything for now.
* You need to create some files with the passwords for the root and IVP user for MySQL.  Follow these steps:
    * Create a secrets folder in your project root.
    * Inside the secrets folder create files mysql_password.txt and mysql_root_password.txt with any arbitrary line of  text inside them.
* Install (if not already present) the **Dev Containers** extension.
* Use ctrl-shift-P (Command Pallette) and select *Dev Containers: Open Folder in Container* and select the current project folder.
* At this point VS Code should start building a container for your V2XHUB development as well as MySQL and PHP containers that are necessary for running the system.

Once a container has been built you should not need to rebuild it again unless something changes in the dependencies or Docker setup.  If you do need to rebuild then use the *Dev Containers: Rebuild Container* command from the pallette.

One of the great features about this setup is that the devcontainer.json file can be configured with a list of extensions to be included any time the container is opener so the list can be configured with what is most commonly used with the project so new users do not need to figure that out by themselves or manually install them.

## Tasks

The configuration comes with 3 configured tasks.  These can be run under from the menu via *Terminal -> Run Task...*.  The tasks are described below.
* build : This builds the code using the build.sh script in the src directory.  The output from this can be using to navigate directly to any problems.  This task is configured as the default build task so it can be activated with the shortcut ctrl-shift-B.
* clean : Cleans out the build files if you need to start over with compiling for any reason.
* run : After a successful build you can use the run task to run your code changes and connect to the system using a browser.

## Database Reset

If you want to reset the database for any reason run these commands outside any container.  The SQL commands that are loaded with the database are only run when there is no database data to start.
```
docker stop mysql
docker rm mysql
docker volume rm v2x-hub_devcontainer_mysql-datavolume
```
Then rebuild the container in VS code.
