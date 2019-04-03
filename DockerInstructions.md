# Running Dockerized V2X Hub

If you are installing V2X Hub using a docker image, you can use the following instructions:

## Install Docker CE and containerd.io

Instructions for installing Docker may change, so please use the current instructions at the Docker website:
https://docs.docker.com/install/linux/docker-ce/ubuntu/

## Run the V2X Hub image

1.  Download the V2X Hub source to your local machine
2.  Navigate to the /configuration/ folder
3.  Update the apt package index:
```
$ sudo apt-get update
```
4.  From the /configuration/ folder, run:
```
$ sudo docker-compose up
```

This will run V2X Hub in a docker contianer on your host ocmputer along with separate containers for mysql and apache-php

## Access the V2X Hub Interface

V2X Hub currently requires a certificate exception to run on a local machine.  Here's how you add that exception.

1.  In your browser, navigate to https://127.0.0.1:19760/
2.  Add an exception to the security certificate requirements in your browser.
3.  Navigate to https://127.0.0.1 for the V2I Hub GUI.  All plugins shipped with the code are installed by default.
