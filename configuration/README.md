## Introduction
This directory contains deployment and configuration instructions for deploying V2X-Hub on both ARM64(arm64) and x86(amd64) architectures.

> [!NOTE]
> Separate deployment files/configurations are no longer necessary for arm64 and x86 deployments.

### Deployment Instructions
Once downloaded, navigate to the configuration directory:
```
cd ~/V2X-Hub/configuration/
```
Run the initialization script:
```
sudo ./initialization.sh
```
Follow the prompts during installation.

You will be prompted to create a mysql_password and mysql_root_password. You may make the passwords whatever you like, but you will need to remember them.
```
Example: ivp
```
You will also be prompted to create a V2X Hub username and password. You may make these whatever you’d like, but will need to use them to log into the web UI. Example:
```
Username: v2xadmin

Password: V2xHub#321
```
You will then need to enter the mysql_password you created in step 5a:
```
Example: ivp
```
After installation is complete, the script will automatically open a web browser with two tabs.

Navigate to the tab labeled as “Privacy Error” and select the “Advanced” button.

Click on “proceed to 127.0.0.1 (unsafe)”

Note: This page will not do anything when clicking proceed

Close the Privacy Error tab and wait for the initial V2X Hub tab to finish loading

Enter the login credentials you created in step 5b and login.

Installation complete!

### Simulation Setup

To support execution in a simulated environment, V2X-Hub is in the process of integrating with CDASim, a Co-Simulation tool built as an extension of Eclipse Mosiac. This extension will incorporate integration with several other platforms including CARMA-Platform and CARLA. The setup for this simply requires setting environment variables for the V2X-Hub docker compose deployment. These can be set via the `initialization.sh` script and can be manually edited after.

* **V2XHUB_VERSION** – Version of V2X-Hub to deloy ( Docker Tag/ GitHub Tag )
* **SIMULATION_MODE** – Environment variable for enabling simulation components for V2X-Hub. If set to "true" or "TRUE" simulation components will be enable. Otherwise, simulation components will not be enabled.
* **KAFKA_BROKER_ADDRESS** – Environment variable for storing Kafka broker connection string (including port).
* **TIME_SYNC_TOPIC** – Environment variable for storing Kafka time sync topic.
* **SIMULATION_IP** – Environment variable for storing IP address of CDASim application.
* **SIMULATION_REGISTRATION_PORT** – Environment variable for storing port on CDASim that handles registration attempts.
* **TIME_SYNC_PORT** – Environment varaible for storing port for receiving time sync messages from CDASim.
* **V2X_PORT** – Environment variable for storing port for receiving v2x messages from CDASim
* **SIM_V2X_PORT** – Environment variable for storing port for sending v2x messages to CDASim
* **V2XHUB_IP** – Environment variable for storing IP address of V2X Hub.
* **INFRASTRUCTURE_ID** – Environment variable for storing infrastructure id of V2X Hub.
* **SENSOR_JSON_FILE_PATH** – Environment variable for storing path to sensor configuration file. This is an optional simulation environment variable that allows for setting up simulated sensor for a V2X-Hub instance. Example file can be found in the **CDASimAdapterPlugin** tests [here](../src/v2i-hub/CDASimAdapter/test/sensors.json).

### Access V2X-Hub 
To access V2X-Hub UI, either chromium or google-chrome browser can be used by running the following commands:
```
chromium-browser --ignore-certificate-errors <v2xhub_ip>
```
or 

```
google-chrome --ignore-certificate-errors <v2xhub_ip>
 ```

> [!NOTE]  
> V2X-Hub Server uses a secure WebSocket connection to communicate with the browser. For this, V2X-Hub Server uses a self-signed certificate instead of one signed by a trusted certificate authority (CA). Most of the browsers do not trust this until explicitly told to do so by navigating to the WebSocket URL (https://<v2xhub_ip>:19760) and accepting the risk. The `--ignore-certificate-errors` option instructs the browser to ignore certificate errors and warnings which removes manual effort of accepting this risk on V2X-Hub deployments. **Using the browser with this option enabled to access the internet or any other application is not advised as it is a security risk**.