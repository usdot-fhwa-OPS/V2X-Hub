V2X-Hub Release Notes
----------------------------

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
