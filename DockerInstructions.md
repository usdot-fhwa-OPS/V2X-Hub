# Running Dockerized V2X Hub

If you are installing V2X Hub using a docker image, you can use the following instructions:

## Install Docker CE and containerd.io

Instructions for installing Docker may change, so please use the current instructions at the Docker website:
https://docs.docker.com/install/linux/docker-ce/ubuntu/

## Docker Hub repository location

Docker Hub repo is located at: https://hub.docker.com/r/usdotfhwaops/v2x-hub

## Run the V2X Hub image

1.  Download the V2X Hub source to your local machine
2.  Update the apt package index:
```
$ sudo apt-get update
```
3. Install lamp-server
```
$ sudo apt-get install lamp-server^
```
if/when prompted, enter a root password: ivp
4. To install database, from the /V2X-Hub/data/DatabaseSetup directory, execute the script using the following commands:
```
$ chmod +x install_db.sh
$ sudo ./install_db.sh
```
5.  To pull the docker image, run the following command:
```
$ sudo docker pull usdotfhwaops/v2x-hub:v2x-hub-v3.2
```
6.  To run dockerized V2X-Hub, run the following command (run without tags for latest build):
```
$ sudo docker run --network=host usdotfhwaops/v2x-hub:v2x-hub-v3.2
```
This will run V2X Hub in a docker contianer on your host computer while using mysql from the host computer.

5.  You can access V2X-Hub using mysql by running the following commands from terminal:
```
$ mysql -uIVP -pivp -DIVP -hlocalhost
```
## Access the V2X Hub Interface

V2X Hub currently requires a certificate exception to run on a local machine.  Here's how you add that exception.

1.  In your browser, navigate to https://127.0.0.1:19760/
2.  Add an exception to the security certificate requirements in your browser.
3.  Navigate to https://127.0.0.1 for the V2I Hub GUI.  All plugins shipped with the code are installed by default.


