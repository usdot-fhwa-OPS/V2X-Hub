V2X-Hub Release Notes
----------------------------
Version 7.2.1, released April 15th, 2022
--------------------------------------------------------
V2X Hub release 7.2.1 includes added functionality for the CARMACloud Plugin to support and a hot fix for positive acknowledgment:
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
