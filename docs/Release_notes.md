V2X-Hub Release Notes
----------------------------

Version 5.0, released March 26th, 2020
--------------------------------------------------------

**Summary:**
V2X-Hub 5.0 is comprised of a number of enhancements as well as bug fixes. The additions primarily consist of the new Preemption, Pedestrian, and TIM plugins, along with new performance metrics.

Enhancements in this release:
- Preemption Plugin - Calls a preemption table on a controller using NTCIP 1202 V3 commands to provide passage to an emergency vehicle upon request through BSMs
- Pedestrian Plugin - Creates a PSM using information obtained from nomadic devices (ex. cell phones) through a local webserver.
- TIM Plugin - Creates and broadcasts a TIM message from an .xml file based on user input through GUI or local webserver
- Performance Metrics - Provides queue volume, spot speed, and trajectory information from connected vehicles to allow precision data about multiple measures (such as signal state and vehicle location) over extended periods of time.
- Updated arm64 and amd64 initialization scripts to accept a user-provide username and password for V2X-Hub

Bug fixes in this release:
- Fixed segmentation fault which caused V2X-Hub to crash when it received BSMs with missing headers.
