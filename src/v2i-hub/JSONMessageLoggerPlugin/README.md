# JSON Message Logger Plugin

## Introduction

The JSON Message Logger Plugin provides functionality to log human readable JSON J2735 Messages in **Tx** and **Rx** logs for J2735 messages transmitted and received by V2X Hub respectively. The log files rotate daily or at 10 mb and are stored under `/var/log/tmx/`.

## Related Plugins

A list of plugins related to the JSON Message Logger Plugin.

### Immediate Forward Plugin

Any J2735 messages sent to an RSU by the Immediate Forward Plugin will be recorded in the **Tx** log.

### Message Receiver

Any J2735 messages received by V2X Hub via the Message Receiver or any other integration plugin, that are not planned for broadcast via the Immediate Forward plugin will be recorded in the **Rx** log.


## Configuration/Deployment

This plugin has no unique configuration parameter.


## Design


### Messages

**J2735**: Any V2X Message defined in the J2735 Specification

## Functionality Testing