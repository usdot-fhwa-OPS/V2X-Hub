# Intersection Validation Documentation

## Introduction

The Intersection Validation Plugin is responsible for validating MAP and SPaT received from RSUs against the CTI 4501 standard.

## Related Plugins

A list of plugins related to the Intersection Validation Plugin.

## Configuration/Deployment

There are no unique configuration parameters for this plugin, although the log level can be set to "Warning" to view any message interval violations, and "Debug" to view the decoded MAP and SPaT messages in a JSON format that undergo message validation. The log level can be set to "Error" to view any errors from the message validation and where in the message the violation exists. 

The JSON schemas, which detail the proper structure and required fields in a MAP and SPaT message, for message validation exist under the resources directory. Both the MAP and SPaT schemas are custom and include additional required fields specified by the CTI 4501 standard, on top of the required fields specified by the J2735 standard. For SPaT messages, those additional fields include:

* TimeStamp

* Timing under state-time-speed

* StartTime

* MaxEndTime

* NextTime

* RoadAuthorityID

## Design

As messages are sent to V2XHub, the IntersectionValidation Plugin currently conducts two forms of validation against the CTI 4501 standard. Firstly, the plugin calculates the intervals in which MAP and SPaT messages are received by V2XHub and validates it against the threshold set by the CTI 4501 standard. A warning is thrown in the Event Logs if the interval for between MAP and SPaT messages are greater than the threshold. Additionally, the plugin validates each SPaT and MAP message that is received against a JSON schema to verify that all required fields set by the CTI 4501 standard are present. Another warning is thrown in the Event Logs if any required field is missing from the received MAP and SPaT message.

### Messages

* **SPaT**: This message contains information from the traffic signal controller about Signal Phase and Timing (SPaT).
* **MAP**: This message contains information detailing the geometric map data of an intersection.

## Functionality Testing