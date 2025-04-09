# Pedestrian Plugin Documentation

## Introduction

This plugin is responsible for generating PSM (Personal Safety Message), SDSM (Sensor Data Sharing Message) and TIM (Traver Information Message) for pedestrians. The data for these messages can come from REST POST calls of PSM to broadcast, FLIR camera detections or a static TIM message sent in configuration

## Related Plugins

A list of plugins related to the Pedestrian Plugin.

### Immediate Forward Plugin

For RSU Immediate Message Forwarding (IMF) functionality forward PSM, SDSM or TIM 

## Configuration/Deployment

This plugin has several configuration parameters. Below these are listed out as together with descriptions on how to set them.


## Design


### Messages

**PSM**: Personal Saftey Message
**SDSM**: Sensor Data Sharing Message
**TIM**: Traveler Information Message
## Functionality Testing

Testing the functionality of a configured instance of the Pedestrian plugin simply follow the steps outlined below

1) Call `python3 -m pip install -r ./scripts/requirements.txt`
2) Run the test script to mock FLIR camera detections `python3 ./scripts/websocketServer.py` or the script to mock sending REST POST requests `python3 ./scripts/sendPsm.PY`
