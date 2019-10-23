# Running Dockerized V2X Hub for AMD based systems

If you are running V2X Hub using a docker image, you can use the following instructions:

## Install Docker CE

Instructions for installing Docker may change, so please use the current instructions at the Docker website:
https://docs.docker.com/install/linux/docker-ce/ubuntu/

## Install Docker compose

Instructions for installing Docker may change, so please use the current instructions at the Docker website:
https://docs.docker.com/compose/install/

## Docker Hub repository location

Docker Hub repo for v2x-hub is located at: https://hub.docker.com/r/usdotfhwaops/v2x-hub

Docker Hub repo for mysql is located at: https://hub.docker.com/r/usdotfhwaops/v2x-hub

## Run the V2X Hub image
```
$ cd /V2X-Hub/configuration/
$ sudo docker-compose up
```

## Access the V2X Hub Interface

V2X Hub currently requires a certificate exception to run on a local machine.  Here's how you add that exception.

1.  In your browser, navigate to https://127.0.0.1:19760/
2.  Add an exception to the security certificate requirements in your browser.
3.  Navigate to https://127.0.0.1 for the V2I Hub GUI.  All plugins shipped with the code are installed by default.
