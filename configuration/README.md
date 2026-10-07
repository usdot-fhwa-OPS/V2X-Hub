## Introduction
This directory contains deployment and configuration instructions for deploying V2X-Hub on both ARM64(arm64) and x86(amd64) architectures with enhanced network security.

> [!NOTE]
> Separate deployment files/configurations are no longer necessary for arm64 and x86 deployments.

> [!IMPORTANT]
> **Network Security Enhancement**: V2X-Hub now uses isolated Docker networks for improved security. The database is completely isolated from external access, and only necessary ports are exposed. See [NETWORK_SECURITY.md](NETWORK_SECURITY.md) for detailed information.

## Database Configuration

V2X-Hub uses a secure network architecture with the following database configuration:

- **Database Host**: `db` (Docker service name for internal communication)
- **Database Port**: `3306` (accessible only within internal network)
- **Database Name**: `IVP`
- **Database User**: `IVP`
- **Password Management**: Secured via Docker secrets

The database is completely isolated on the `v2xhub_data_internal` network and cannot be accessed directly from external networks, providing enhanced security.

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

You will be prompted to create a mysql_password. This is the password to the MySQL configuration database for the `IVP` user. Please create a unique and secure password and remember it
```
Example: ivp
```
You will also be prompted to create a V2X Hub username and password. You may make these whatever you’d like, but will need to use them to log into the web UI. Example:
```
Username: v2xadmin

Password: Changeme123!
```

After installation is complete, the script will automatically open a web browser with two tabs.

Enter the login credentials you created in the steps above and login.

Installation complete!

### Simulation Setup

To support execution in a simulated environment, V2X-Hub is in the process of integrating with CDASim, a Co-Simulation tool built as an extension of Eclipse Mosiac. This extension will incorporate integration with several other platforms including CARMA-Platform and CARLA. The setup for this simply requires setting environment variables for the V2X-Hub docker compose deployment. These can be set via the `initialization.sh` script and can be manually edited after.

### Docker Environment Variables

This section covers environment variables used to configure V2X Hub deployment

#### General System Environment Variables

This category of environment variables configures the general V2X Hub system.

* **V2XHUB_VERSION** – Version of V2X-Hub to deploy ( Docker Tag/ GitHub Tag )
* **V2XHUB_IP** – Environment variable for storing IP address of V2X Hub. Defaults to 0.0.0.0
> [!NOTE]
> For docker compose deployments please use default value to accomodate docker bridge network security setup. For non-containerized or custom deployments set this value to the IP address of the hosting machine.
* **INFRASTRUCTURE_ID** – Environment variable for storing infrastructure id of V2X Hub.
* **V2XHUB_USER** – V2X Hub Administrator Username to create on startup
* **V2XHUB_PASSWORD** – V2X Hub Administrator Password to create on startup
* **MYSQL_HOST** – Database hostname (e.g. Docker service name, db or IP Address for database)
* **MYSQL_PORT** – Database port (internal only)
* **MYSQL_DATABASE** – Database Name
* **MYSQL_USER** – Database username
* **MYSQL_PASSWORD** – Managed via Docker secrets
* **V2XHUB_VOLUME_PATH** – Path to which local volume or shared memory between the container and host machine will be setup. See the docker-compose.yml for more information on the specific volumes V2X Hub deployets.

#### Simulation Docker Environment Variables

This category of environment variables configures V2X Hub for simulation and is only relevant in simulation deployments 
* **SIMULATION_MODE** – Environment variable for enabling simulation components for V2X-Hub. If set to "true" or "TRUE" simulation components will be enable. Otherwise, simulation components will not be enabled.
* **TIME_SYNC_TOPIC** – Environment variable for storing Kafka time sync topic.
* **SIMULATION_IP** – Environment variable for storing IP address of CDASim application.
* **SIMULATION_REGISTRATION_PORT** – Environment variable for storing port on CDASim that handles registration attempts.
* **TIME_SYNC_PORT** – Environment variable for storing port for receiving time sync messages from CDASim.
* **V2X_PORT** – Environment variable for storing port for receiving v2x messages from CDASim
* **SIM_V2X_PORT** – Environment variable for storing port for sending v2x messages to CDASim
* **SIM_LOCATION_X** – Environment variable for storing X Coordinate in OSM file for V2X Hub location (m)
* **SIM_LOCATION_Y** – Environment variable for storing Y Coordinate in OSM file for V2X Hub location (m)
* **SIM_LOCATION_Z** – Environment variable for storing Z Coordinate in OSM file for V2X Hub location (m)


* **SENSOR_JSON_FILE_PATH** – Environment variable for storing path to sensor configuration file. This is an optional simulation environment variable that allows for setting up simulated sensor for a V2X-Hub instance. Example file can be found in the **CDASimAdapterPlugin** tests [here](../src/v2i-hub/CDASimAdapter/test/sensors.json).

#### Host Port Mapping Docker Environment Variables

All incoming communication to V2X Hub requires Host to Container port mapping since V2X Hub, in the containerized deployment is deployed into a docker virtual internal network. This is for security reasons to prevent unnecessary access to V2X Hub internal communication from host machine. This requires almost all incoming V2X Hub communication to include configurations host to container port mapping. The V2X Hub standard practice for these is to use docker environment variables and to name them accordingly

```
<V2XHUB_PLUGIN_NAME>_EXTERNAL_<PORT_NAME>_PORT
```
OR
```
<SERVICE_NAME>_EXTERNAL_<PORT_NAME>_PORT
```
> [!NOTE]  
> When incoming connections are from docker containers running on an accessible container, host port mapping is unnecessary. This is the case for CDASim incoming communication


> [!NOTE] 
> Setting these external ports to 0 will allow docker to find an open port on the host machine and just use that. This avoids port conflicts, which is especially useful when running multiple instances of V2X Hub (scaling)

Below is a list of currently available external port configurations: 
**PHP_EXTERNAL_HTTP_PORT** : HTTP access to web UI 
**PHP_EXTERNAL_HTTPS_PORT** : HTTPS access to web UI
**COMMAND_PLUGIN_EXTERNAL_PORT** : Connection between browser and V2X Hub server
**MESSAGE_RECEIVER_PLUGIN_EXTERNAL_PORT** : Incoming UPER encoded V2X messages for V2X Hub to process (e.g from RSU)
**SPAT_PLUGIN_EXTERNAL_PORT** : Incoming SPAT data from Traffic Signal Controller (TSCBM or UPER SPAT)
**TIM_PLUGIN_EXTERNAL_PORT** : TIM Plugin REST API for receving XER encoded TIM messages to broadcast
**CARMA_CLOUD_EXTERNAL_PORT** : CARMA Cloud Plugin REST API for receiving communication from CARMA Cloud
**MUST_SENSOR_PLUGIN_EXTERNAL_PORT** : Must Sensor Plugin Port for receiving detections from MUST sensor.
**PORT_DRAYAGE_SERVICE_EXTERNAL_HTTP_PORT** : HTTP access to Port Drayage Web UI 

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
> V2X-Hub initialization script uses [mkcert](https://github.com/FiloSottile/mkcert), a simple tool for making locally-trusted development certificates for HTTPS communication and placing them in the `${V2XHUB_VOLUME_PATH}/ssl/` directory. For deployment, it is recommended that you generate your own trusted certificates from a real certificate authorities (CAs). MKCert can also be used to setup a local CA but that is up to deployers.

> [!NOTE]  
> If no certificates are present at start-up time, the V2X Hub container will create self signed certificates using `openssl` (see `container/generate_certificates.sh`). These certificates need to be explicitly trusted by the browser. To do this simply navigate to `https://<v2xhub-ip>` and accept the warning. After this you should be redirected to the login page.

