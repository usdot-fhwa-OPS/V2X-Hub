# Running Dockerized V2X Hub for ARM based systems

If you are running V2X Hub using a docker image, you can use the following instructions:

## Run the V2X Hub image
```
$ cd /V2X-Hub/configuration/arm64
$ chmod +x initialization.sh
$ sudo ./initialization.sh
```

## Access the V2X Hub Interface

V2X Hub currently requires a certificate exception to run on a local machine.  Here's how you add that exception.

1.  In your browser, navigate to https://127.0.0.1:19760/
2.  Add an exception to the security certificate requirements in your browser.
3.  Navigate to https://127.0.0.1 for the V2I Hub GUI.  All plugins shipped with the code are installed by default.
