V2X-Hub Release Notes
---------------------------------

Version 7.8.0, released Aug 26th, 2024
--------------------------------------------------------

**Summary:**
V2X Hub release 7.8.0 includes significant enhancements such as a new telematics module for streaming V2X Hub data into the telematics server and Influx database, and an RSU Health Monitor to stream J2735 data from V2X Hub to the cloud in near real-time. The telematics plugin now subscribes to all TMX messages from V2X Hub, improving data integration. Additionally, this release addresses key issues, such as fixing the telematics bridge memory usage and correcting the handling of null topics, ensuring accurate data management and system reliability.

Enhancement in this release:  

- V2X-Hub PR 564: Implemented a RSU Health Monitor Plugin to directly interface with RSUs connected to V2X Hub via SNMP protocol. 
- V2X-Hub PR 565: Implemented a telematics plugin to subscribe to all TMX messages from V2X Hub. 
- V2X-Hub PR 567: Implemented the telematic bridge to stream J2735 data from V2XHub to the cloud in near real-time. This includes forwarding JSON messages, 
  registering with WFD cloud services, mapping TMX message types to topics, and streaming requested TMX messages in JSON format. 
- Issue 591: Created a telematics module to stream V2X Hub data into the telematics server and Influx database. This includes connecting to V2X Hub, 
  subscribing/unsubscribing to topics, and streaming data to the telematics tool. 
- V2X-Hub PR 599: Fixed memory usage issue in the telematics bridge, which increased indefinitely when forwarding messages to the telematics tool. 
- V2X-Hub PR 606: Updated the RSU Health Monitor plugin to support monitoring the health status of multiple RSUs per V2X Hub instance. 
- V2X-Hub PR 613: Added configuration parameters for RSU Health Monitor Plugin to identify the source of payload when multiple RSUs are connected.
  
Fixes in this release: 

- N/A

Version 7.7.0, released Aug 15th, 2024
--------------------------------------------------------

**Summary:**
V2X Hub release 7.7.0 introduces key enhancements and fixes to improve data streaming, upgrade MySQL to version 8.0, and support multi-architecture Docker deployments. This release includes updates to initialization scripts and addresses clock reference issues in simulation mode. Additionally, this release removes obsolete Docker tags and makes the SENSOR_JSON_FILE_PATH an optional environment variable to enhance flexibility in simulation setups.

Enhancement in this release:  

- V2X-Hub PR 612: Updated MySQL to 8.0 to support multi-architecture Docker deployment, removed architecture-based configuration folders, and updated initialization scripts. 
- V2X-Hub PR 611: Updated TCR message oldest field to use data sent by carma-platform in simulation mode to address clock reference issues. 
- V2X-Hub PR 615: Updated the initialization script to generate a .env file based on user input. 

Fixes in this release: 

- V2X-Hub PR 614: Removed the "version:" tag at the top of the docker-compose files. These are no longer needed and are obsolete in current and future versions of Docker. 
- V2X-Hub PR 616: Updated SENSOR_JSON_FILE_PATH as optional environment variable in simulation since spawning sensors is not required for the base line functionality of V2X-Hub. Before this update CDASimAdapter would accept empty SENSOR_JSON_FILE_PATH files but not missing ones. 


Version 7.6.0, released April 10th, 2024
--------------------------------------------------------

**Summary:**
V2X Hub release 7.6.0 includes functionality for integrating V2X-Hub with simulated sensors in CDASim. CDASim can now publish detection data to V2X-Hub for ingestion. Furthermore, this release added a functionality to encode/decode J3224 Sensor Data Sharing Message (SDSM) and forward these messages between CARMA-Streets and CDASim for Vulnerable Road User (VRU) Cooperative Perception testing in simulation. Unrelated improvements include initial integration with telematics tool and general system improvements.

**<ins>V2X Hub CDASim Functionalities </ins>**

Enhancements in this release: 

- Issue 549/PR 550: Moved Kafka time producer from CDASimAdapter to CARMA-Streets plugin, and updated CARMA-Streets Plugin to be simulation time aware. CDASim Adapter was previously forwarding time sync messages directly to CARMA-Streets. Improved CARMA Streets Plugin to extend our PluginClientTimeAware class to make it aware of simulation.  
- Issue 590: Add CDA Sim simulated sensor integration. Allow V2X-Hub to consume detection data from simulated sensor in CDASim. 
- Issue 589: Implement support for CARMA-Street SDSM functionality. This includes adding SDSM encoding/decoding and message forwarding between CARMA-Streets and CDASim 

Fixes in this release: 

- Issue 538 & Issue543 & PR 547: Fixed CARMA Streets Plugin Kafka Consumers. 1) Removed the Kafka consumer/producer initialization during config parameter update. 2)  Replaced consumer creation with v2xhub kafka_client library. 3) Added producer creation in kafka_client library.  
- PR 12 (not V2X Hub Repo): Fixed wait_for_initialization for multiple threads  

Other Enhancements: 

- PR 545: Added PSID metadata to BSMs and SRMs that are sent to the Message Receiver plugin. If RouteMessage is enabled, the messages will be available for the Immediate Forward plugin to use 
- PR 592 / Issue 593: Update initialization script to install necessary dependencies, create secret files, prompt user to enter MySQL passwords, and create v2xhub user and open web browser to correct URL. 
- Issue 591: V2X-Hub Telematics tool integration. Allows V2X-Hub to stream data to telematics tool for data visualization. Includes development of new RSU Health Monitoring Plugin. 
- PR 580: Added a copy command from the dependency container to the final container for /usr/local/include/ which ensures the header files get copied to the final container. 

Known issues related to this release: 

- Issue 540: CDASim Time Synchronization is non-time-regulating. If simulation runs too fast (faster than real-time) for V2X Hub to keep up, V2X Hub can fall behind in time. 
- Issue 507: SPaT plugin throws segfault when in SIM MODE. 

Version 7.5.1, released June 21st, 2023
--------------------------------------------------------

**Summary:**
 V2X Hub release 7.5.1 includes added functionality to integrate V2X Hub with CDASim environment. This integration includes V2X Hub registering as a Roadside Unit (RSU) in the CDASim environment, consuming and producing J2735 messages to the CDASim environment, and adding functionality to synchronize plugins to CDASim simulation time.

**<ins>V2X Hub CDASim Functionalities </ins>**

Enhancements in this release: 

- Added new carma-time-lib to V2X Hub to allow services to use an external source for time value and update rate. 
- Added new Plugin Client ClockAware, which extends Plugin Client and implements functionality to consume time sync messages and updates a carma-clock object from carma-time-lib. Any plugins that want to synchronize their time to simulation must extend this plugin to gain access to this functionality. 
- Added CDASim Adapter plugin which is responsible for establishing connection between V2X Hub and CDASim environment. This includes a handshake that provides information about the V2X Hub simulated location and ID and message forwarding for J2735 messages and time synchronization messages. This plugin requires several environment variables to be set which are documented on the GitHub repo README.md. 

Fixes in this release: 

- PR 488: Added a simulated clock functionality with the new time library and tested. 
- PR 489: Setup Kafka consumers for the time topic when running in simulation mode. 
- Issue 492: Created a carma-simulation adapter shell for service that will act as adapter for CARMA Simulation integration. 
- PR 509: Added a V2X Hub plugin inside the simulation platform to receive all messages from V2X Hub. This plugin contains parameters and variables that are provided in real-world scenarios. 
- Issue 514: Added handshake functionality to carma-simulation ambassador instance which register’s the V2X Hub instance inside the simulator to allow multiple V2X Hub instances to connect with a single CARMA Simulation platform. 
- Issue 535: Updated infrastructure registration to use a cartesian point as location over a geodetic point to allow for easier configuration of simulated location of an RSU. 
- Issue 537: Fixed configuration parameters to correctly map X, Y, Z coordinates to Point for Infrastructure registration in CDASim Adapter. 
- Issue 525: Fixed CDASim Adapter plugin that throws an exception while attempting CDASim handshake with CARMA-Simulation. 

Known issues in this release: 

- Issue #540: CDASim Time Synchronization is non-time-regulating. If simulation runs too fast (faster than real-time) for V2X Hub to keep up, V2X Hub can fall behind in time. 
- Issue #507: SPaT plugin throws segfault when in SIM MODE  
- Issue #10 in carma-time-lib (not V2X Hub repo): wait_for_initialization does not support notifying multiple threads Work around exists for services/plugins using carma-time-lib. 
- Issue #543: CARMA Streets Plugin Kafka Consumers can send redundant subscription attempts on initialization and can cause subscriptions to silently fail. 

**<ins>Other </ins>**

Enhancements in this release: 

- Issue 511: Added new functionality get the log time for a message received in V2xHub to forward to Carma cloud, and from receiving in Carma Cloud to forward V2xhub. 

Version 7.5.0, released May 5th, 2023
--------------------------------------------------------

**Summary:**
V2X Hub release 7.5.0 is comprised of the following new features: a new ERVCloudForwardingPlugin to enable BSMs from active Emergency Response Vehicles (ERVs) to be forwarded to CARMA Cloud in support of message forwarding to V2X Hub instances along an ERV’s future route when deployed along with other CARMA tools to demonstrate move-over law when an ERV is approaching a CDA vehicle from behind; new features to support CARMA Simulation integration such as simulation clock functionality; and a newly developed CARMA Simulation adaptor shell and handshake functionality to allow multiple V2X Hub instances to connect with a single CARMA Simulation platform. Along with the above enhancements, several enhancements and bug fixes are included in this release. 

**<ins>Freight Emergency Response Functionalities</ins>**

Enhancements in this release related to Freight Emergency Response: 

- PR 460: The creation of a new ERVCloudForwardingPlugin that enables V2X Hub to register a connected RSU, along with its location information, with CARMA Cloud. Additionally, this plugin is responsible for sending received BSMs from active Emergency Response Vehicles (ERVs) to CARMA Cloud in support of message forwarding to V2X Hub instances located along the ERV’s future route. 

**<ins>Other</ins>**

Enhancements in this release: 

- Issue 262: Updated CARMA Streets plugin to receive and decode Mobility Path messages into JSON through Kafka. 
- PR 486: Updated the V2X Hub docker images to Ubuntu 22 (Jammy) which has LTS support through April 2027. This will also support new libraries created using the Carma-builds project. 
- PR 487: Added some changes to allow for Docker to be installed on different Linux distros for arm64. 

Fixes in this release: 

- Issue 484: Fixed PedestrianPlugin does not update when any configuration changes are made with in Plugin, either when plugin is off or on. 
- PR 494: Sets some error message in the Command Plugin for file upload operations to ERROR instead of DEBUG so they can be seen on the command line by default. 

Version 7.4.0, released Feb 10th, 2023
--------------------------------------------------------

**Summary:**
V2X Hub release 7.4.0 includes the following: added functionality changes for CARMA-Streets plugin to broadcast SPaT messages with future event information (Movement Event List) and updated scheduling message logic for cooperative driving automation (CDA) transportation systems management and operations (TSMO) use cases (UCs) UC1 (stop-controlled intersection in a CDA environment) and UC3 (adaptive traffic signal optimization in a CDA environment) . The MAP Plugin functionality is updated to provide processing of a new '.uper' format MAP message which will expect hex encoded MAP messages including the message frame. Updated V2Xhub plugin names in UI. V2xhub Core to include BSM Part II with extension that has destination points. Along with the above enhancements, several bug fixes and CI related enhancements are included in this release. 

Enhancements in this release:
- Issue 398: Added functionality to Map Plugin to process files with extension '.uper' which will expect hex encoded MAP messages including the message frame. Also resolves some issues encountered with processing other formats of MAP messages. 
- Issue 407:  Updated CARMA-Streets Plugin to consume modified spat JSON data from CARMA-Streets, convert it to J2735 SPaT messages, and broadcast these messages. 
- Issue 412: Updated CARMA-Streets Plugin to consume UC 3 scheduling messages that only include ET (entering time) and read intersection type from scheduling message. Also updated UC3 and UC1 scheduling messages to include intersection ID in Mobility Header. 
-	Issue 422: Updated V2X-Hub Plugin names in UI as below:
      1.	DSRCImmediateForwardPlugin -> ImmediateForwardPlugin
      2.	ODELoggerPlugin -> ODEForwardPlugin
      3.    Merged the SPaTLoggerPlugin into the MessageLoggerPlugin 
      4.    Removed the ODEPlugin to only maintain the ODEForwardPlugin 
-	Issue 423&425: Updated V2X-Hub Class names as below:
      1.	DSRCImmediateForwardPlugin -> ImmediateForwardPlugin
      2.	ODEForwardPlugin -> ODELoggerPlugin
      3.	MessageLoggerPlugin -> MessageLoggerPlugin
-	Issue 427: Updated plugins to allow different channels for message file format and this channel configuration covers all messages being generated by V2X Hub.

Fixes in this release:

- Issue 351: Fixed PHP UI missing Configuration parameters due to the configuration parameter values stored in the localhost.sql files. 
- Issue 415: Fixed CARMA-Streets Plugin incorrectly translates BSM IDs with leading zeros. 
- Issue 420:  Fixed CARMA-Streets plugin to keep track of any invalid SPaT messages it receives and skip invalid messages to continue processing incoming valid data. 
- Issue 424: Update V2X-Hub UI Version numbers with most recent numbers. 
- Issue 446: Fix Spat Plugin Socket Handling

Version 7.3.1, released July 29th, 2022
--------------------------------------------------------
**Summary:**
V2X Hub release version 7.3.1 is a hotfix release for 7.3.0. The fixes primarily occurred during the Implementation of IHP2 Speed Harmonization algorithm in Carma-cloud application.

Bug fixes in this release:
 - Issue 392: Fixed Large latencies experienced between V2XHub receiving a Traffic Control Request (TCR) and broadcasting corresponding Traffic Control Messages (TCMs).
 - Issue 404: Fixed V2xhub cannot encode the TCM if the package detail has minplatoonhdwy tag.


Version 7.3.0, released June 14th, 2022
--------------------------------------------------------
**Summary:**
V2X Hub release 7.3.0 includes added functionality for subscribing to FLIR camera for Pedestrian tracking and PSM broadcast to vehicles in the Pedestrian Plugin. The new FLIR functionality subscribes to the websocket output of the FLIR tracking feed and will generate a PSM for each new track the camera picks up. To use new FLIR websocket data, set `DataProvider` for the PedestrianPlugin to FLIR and configure the `WebSocketHost` and `WebSocketPort` for the FLIR camera. Additional you must set the `FLIRCameraRotation` in degrees, which is a measure of the camera's rotation from true north. To use the REST PedestrianPlugin functionality simply set the `DataProvider` to PSM.

Enhancements in this release:
 - Issue 345: Added Websocket client to consume FLIR data and publish PSM
----------------------------
Version 7.2.3, released May 19th, 2022
--------------------------------------------------------
**Summary:**
V2X Hub release 7.2.3 includes added hot fix for MobilityRequest and MobilityResponse ASN1 Compiler generated encoding/decoding code for Voices project.

Bug fixes in this release:
- Issue 372: After getting the dsrc messages from a vehicle, the v2xhub message receiver is able to pass it to other plugins. However, the other plugins experience decoding issues trying to filter MobilityRequest and MobilityResponse messages.

Enhancements in this release:
 - Issue 369:Add CircleCI workflow that is triggered on numeric github tags on only the master branch. This workflow will be dependent on the build and push workflows for both ARM and AMD images. The workflow will then pull, retag with the github tag name, and push release images.

------------------------------
Version 7.2.2, released May 9th, 2022
--------------------------------------------------------
**Summary:**
V2X Hub release 7.2.2 includes added Hotfix for CARMACloud Plugin to configure repeated TCM broadcast. V2xhub can control the number of times each TCM being repeatedly broadcast upon receiving TCMs from carma-cloud until an TCM acknowledgement is received from the vehicle. After waiting for a configurable time duration to receive TCM acknowledgement, it sends a time out message to carma-cloud notifying that no response from CMV for TCMs. The no response message will be displayed as a warning on the v2xhub admin user interface. If V2xHub receives an acknowledgement from CMV, it will stop broadcasting TCMs and display the acknowledgement on the v2xhub admin user interface and sends the acknowledgement to carma-cloud:

Bug fixes in this release:
- Issue 364: Removing existing TCMs when receiving multiple TCRs with same request ID.Add time out logic for each TCR upon repeatedly broadcast associated TCMs.Fix time out logic that cause segmentation fault.Add configuration parameter to update thread sleep time.Add configuration parameter and logic to control the number times that TCMs can repeatedly broadcast.For each acknowledgement it receives from CMV, it only removes one TCM from the list.
----------------------------
Version 7.2.1, released April 15th, 2022
--------------------------------------------------------
V2X Hub release 7.2.1 includes added functionality for the CARMACloud Plugin to support a hot fix for positive acknowledgment:
- Issue 352: Fix reason field data to display complete reason information.

Version 7.2, released April 12th, 2022
--------------------------------------------------------

**Summary:**
V2X Hub release 7.2 includes added functionality for the CARMACloud Plugin to support:
- Issue 348:Receiving vehicle TCM acknowledgement messages and displaying appropriate positive/negative/no acknowledgement messages on V2X-Hub web UI.
- Issue 348:Fowarding TCM acknowledgement message (negative/no) to CARMACloud.

Enhancements in this release:
- Issue 328: Upgrade base image for V2X-Hub to ubuntu 20
- Issue 349: Added example BSM plugin to show how to use PluginClient's BroadcastMessage message to broadcast BSM
- Issue 350: Added MobilityRequest and MobilityResponse ASN1 encoding and decoding 

Bug fixes in this release:
- Issue 337: fixed add user script
- Issue 310: fixed possible Spat memory leak
- Issue 322: Fixed php and port_drayage_web_service ARM image builds

Version 7.1, released Feb 3rd, 2022
--------------------------------------------------------

**Summary:**
V2X Hub release 7.1 includes a CARMA streets plugin for the following operations:
- Receive, decode and forward the BSM, Mobility Operations Message and Mobility Path Message to CARMA Streets.
- Broadcast schedule plan using Mobility Operations Message received from CARMA Streets.

Enhancements in this release:
- Issue 262: Updated CARMA Streets plugin to receive and decode Mobility Path messages into JSON through Kafka.
- Issue 271: Added Filter Message for BSM and Mobility Operation messages. Publish the decoded BSM and Mobility Operation messages to two different Kafka topics.
Subscribe to Kafka topic to get scheduling plan from CARMA streets. Create a new Mobility Operation message and fill it with scheduling plan data.
Added functionalities to encode Mobility Operation messages, and send it to DSRC manager for broadcast. 
- Issue 317: Updated message receiver plugin to disable the J2735 message signature verification by default we allow users to start up the system with default configurations also go through the setup for this security feature.

Bug fixes in this release:
- Issue 264: Fixed Dependencies Issues, added Carma-streets related data in SQL and Updated plugin install and Docker file to include Carma-streets plugin.
- Issue 314: Fixed few bugs for Carma-street Plugin as below:
     1.	Logging to PLOG statements
     2.	Consolidate logging statements to decrease cognitive complexity
     3.	Added message types to manifest JSON
     4.	Removed old Docker-compose files
     5.	Added CARMA Streets Plugin to sonar analysis
- Issue 289: Fixed ET and DT order in the strategy parameters in Carma-streets Plugin CPP file.

Version 7.0, released Dec 29th, 2021
--------------------------------------------------------

**Summary:**
V2X Hub 7.0 includes new security features to integrate SoftHSM into V2X Hub. Two V2X Hub plugins have been updated to support message verification and signing for SPaT, MAP and BSM messages.

Enhancements in this release:
- Issue 278: The following Security changes have been made to the V2xHub code bases.
    1.The Message Receiver and Immediate Forward plugins have the security features added. This allows any messages to and from RSU to be both signed and verified.
    2.The V2X Hub Admin portal can be used to enable and disable security for the two plugins.
    3.The SoftHSM implementation requires base64 formatted messages for signing or verification requests

Fixes in this release:
-	Issue 253: Fixed sonar scan memory leak issues for preemption plugin.

Version 6.3, released Dec 17th, 2021
--------------------------------------------------------

**Summary:**
V2X Hub release version 6.3 is a hotfix release for 6.0.

Enhancements in this release:
-	Issue 290: Implemented several UI changes to the Port Drayage Web Service for CARMA-Freight as below.
1.	Added landing page with CARMA-Streets Logo and Description
2.	Added Tab Icons and tab notifications
3.	Added Footer with FHWA and MARAD logos
4.	Added indication of operation area (PORT or STAGING) 

Fixes in this release:
-	Issue 287: Updated image tags for arm64 and amd64 Docker-compose deployments to use 6.2 release images instead of release candidate images.

Version 6.2, released Dec 10th, 2021
--------------------------------------------------------

**Summary:**
V2X Hub release version 6.2 is a hotfix release for 6.0.

Enhancements in this release:
- Issue 268: Added Spring Boot Port Drayage Web Service which includes Open API, REST API and Implemented Spring boot REST server. Added Maven pom.xml for Open API Spring boot server code generation. Updated Maven pom.xml for unit test and Jacoco unit test coverage report.
-	Issue 255: Created Lightweight V2X-Hub Deployment Image by removing unnecessary apt-get tools and libraries installed and unnecessary files included in build context and any steps not necessary for V2X-Hub deployment.
-	Issue 272: Added REST Client to Port Drayage Plugin. Changes consist of Qt REST ext/pdclient library generated using Open API code gen and added logic to request and poll loading/unloading and inspection actions from web service, and added Port Drayage Plugin in sonar scanner properties.
-	Issue 279&281: Implementing Port Drayage Plugin CI/CD integration by Adding Docker file, Circle CI build and Push workflow and Docker-compose setup for Port Drayage Web service and updated DSRC Immediate Forward Plugin Configurations to include Mobility Operation message by default.
-	Issue 282: Added Clear button for web service to clear existing actions to allow for repeated executions of the same sequence of actions without requiring restart of web service.

Fixes in this release:
-	Issue 213: Fixed PLOG and FILE_LOG logic where PLOG never prints statements and FILE_LOG only prints error level statements.  Discovered and fixed some segmentation faults in Qt HTTP client code and Tmxctl CLI calls to obtain correct MySQL credentials.
-	Issue 257: Fixed DB Connection Pool Logging to debug the Docker secrets deployment implementation was unintentionally kept in V2X-Hub which cause V2X-Hub to print the MySQL password it uses every time it attempts to authenticate to MySQL.
-	Issue 260: Fixed Sonar scan issues found in Preemption Plugin during V2xhub sonar scanning.
-	Issue 280: Fixed the mobility operation messages used for Vehicle and Infrastructure communication does not indicate the origin of the message where infrastructure may attempt to respond to other infrastructure communication.

Version 6.1, released Oct 15th, 2021
--------------------------------------------------------

**Summary:**
V2X Hub release version 6.1 is a hotfix release for 6.0.

Fixes in this release:
-	Issue 233: Fixed SPaT message to report green status as "permissive" vs "protected" correctly.
-	Issue 124: Fixed issue with interpreting when the signal controller is in flash state.
-	Issue 245: Added configurable parameter for Carma cloud plugin to use for the “oldest” parameter in the TCR request to get controls created within the time period specified by this field in days.
-	Issue 247: Added TCM PSID to messages forwarded to RSU for broadcast.
-	Issue 186: Fixed sonar-scanner settings with code coverage to generate the coverage metrics in sonar cloud.
-	Issue 201: Update docker-compose to manage sensitive data like mysql username and password with Docker secrets.

Version 6.0, released July 30th, 2021
--------------------------------------------------------

**Summary:**
V2X-Hub 6.0 is comprised of a number of enhancements as well as bug fixes. The additions primarily consist of the new Carma Cloud Plugin, ODE Logger Plugin, Spat logger plugin and Validation tool, along with new performance metrics.

Enhancements in this release:
- Carma Cloud Plugin- Added a new plugin for Carma cloud implementation through V2X Hub.
- SPaT logger Plugin – Added SPaT logger plugin to log the incoming message SAE J2735 SPaT in Binary and Json files and included a script which regularly sends captured SAE J2735 SPaT messages to ODE.
- Added password confirmation - PR adds password confirmation to initialization scripts in ARM and AMD.
- ODE Logger Plugin – Added ODE Logger plugin with Kafka Functionality to V2X-Hub designed to stream real-time messages to ODE via the message broker Apache Kafka.
- Validation Tool(Message Validation and Certification Tool v1.0) – The tool follows parallel to the methodology used to define how Intelligent Transportation Systems and Connected and Automated Vehicle message/applications are going to be validated in different ITS equipment’s.
- Message Logger Plugin - Updated Message Logger Plugin to log the incoming message SAE J2735 BSM's in Binary and Json files and included a script which regularly sends captured SAE J2735 BSM's to ODE.
- Reworded and updated links in Docker documentation combined the AMD and ARM documentation documents into one, and updated the links to V2X-hub and MySQL. Updated the link to this new document in the readme.

Bug fixes in this release:
- Fixed ODE Logger plugin bugs with Dockerfile by adding date library install. Also fixed ODE Logger plugin configuration persistence issue by connection to Kafka in update Configuration method.
- Fixed Preemption Plugin - Previously vehicle ID Preemption Plugin was only processing one element of the BSM->coreData.id.buf char array. Added logic similar to Message Logger to read all elements of the char array into an int.
- Tim Plugin and Pedestrian plugin REST endpoint fix - Tim Plugin and Pedestrian plugin did not respond to correct or incorrect POST requests. Added code to both plugins to catch, previously uncaught xml parsing exceptions and return 201 responses for successful requests and 400 responses for incorrect requests.
- Fixed Request ID mismatch issue between TCR and TCM messages in CARMA Cloud Plugin.
- Bug fix Sonar scan - Fixed sonar scan node JS version not found issue. Updated sonar scan session token.
- Message Logger Plugin - Fixed Message Logger Plugin log file generation with updating plugin version numbers and removed some configuration parameters.
- SPaT and BSM binary log file bugs - Fixed SPaT and Binary log files sent to ODE were not being properly processed due to slight header length.
- TIM Plugin bugs - Fixed the TIM plugin was improperly parse start and end date logic and a segmentation fault was causing the plugin to deconstruct.
- Validation tool - Fixed Packet data was not being retrieved from remote RSUs and certain setup instructions were unclear.

Version 5.0, released March 26th, 2020
--------------------------------------------------------

**Summary:**
V2X-Hub 5.0 is comprised of a number of enhancements as well as bug fixes. The additions primarily consist of the new Preemption, Pedestrian, and TIM plugins, along with new performance metrics.

Enhancements in this release:
- Preemption Plugin - Calls a preemption table on a controller using NTCIP 1202 V3 commands to provide passage to an emergency vehicle upon request through BSMs
- Pedestrian Plugin - Creates a PSM using information obtained from nomadic devices (ex. cell phones) through a local webserver.
- TIM Plugin - Creates and broadcasts a TIM message from an .xml file based on user input through GUI or local webserver
- Performance Metrics - Provides queue volume, spot speed, and trajectory information from connected vehicles to allow precision data about multiple measures (such as signal state and vehicle location) over extended periods of time
- Updated arm64 and amd64 initialization scripts to accept a user-provide username and password for V2X-Hub

Bug fixes in this release:
- Fixed segmentation fault which caused V2X-Hub to crash when it received BSMs with missing headers.
- Fixed remove plugin button to function as expected
