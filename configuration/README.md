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
./initialization.sh
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
* **V2XHUB_USER** – V2X Hub Administrator Username to create on startup
* **V2XHUB_USER** – V2X Hub Administrator Password to create on startup
* **SENSOR_JSON_FILE_PATH** – Environment variable for storing path to sensor configuration file. This is an optional simulation environment variable that allows for setting up simulated sensor for a V2X-Hub instance. Example file can be found in the **CDASimAdapterPlugin** tests [here](../src/v2i-hub/CDASimAdapter/test/sensors.json).

### Access V2X-Hub 
To access V2X-Hub UI, either chromium or google-chrome browser can be used by running the following commands:
```
chromium-browser <v2xhub_ip>
```
or 

```
google-chrome  <v2xhub_ip>
 ```

> [!NOTE]  
> V2X-Hub initialization script uses [mkcert](https://github.com/FiloSottile/mkcert), a simple tool for making locally-trusted development certificates for HTTPS communication and placing them in the `.ssl/` directory. For deployment, it is recommended that you generate your own trusted certificates from a real certificate authorities (CAs). MKCert can also be used to setup a local CA but that is up to deployers.
