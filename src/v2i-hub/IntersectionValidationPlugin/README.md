# Intersection Validation Documentation

## Introduction

The Intersection Validation Plugin is responsible for validating MAP and SPaT received from RSUs against the CTI 4501 standard.

## Related Plugins

A list of plugins related to the Intersection Validation Plugin.

## Configuration/Deployment

## Design

As messages are sent to V2XHub, the IntersectionValidation Plugin currently conducts two forms of validation against the CTI 4501 standard. Firstly, the plugin calculates the intervals in which MAP and SPaT messages are received by V2XHub and validates it against the threshold set by the CTI 4501 standard. A warning is thrown in the Event Logs if the interval for between MAP and SPaT messages are greater than the threshold. Additionally, the plugin validates each SPaT and MAP message that is received against a JSON schema to verify that all required fields set by the CTI 4501 standard are present. Another warning is thrown in the Event Logs if any required field is missing from the received MAP and SPaT message.

### Messages

**SPaT**: This message contains information from the traffic signal controller about Signal Phase and Timing (SPaT).
**MAP**: This message contains information detailing the geometric map data of an intersection.

## Functionality Testing