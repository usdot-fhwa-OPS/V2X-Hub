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

### Tolling messages (TAM, TUM, TUMack)

With the J2735 2024 build (the default), V2X Hub recognizes the tolling messages from SAE J3217: Toll Advertisement
(TAM, ID 37), Toll Usage (TUM, ID 38) and Toll Usage Acknowledgment (TUMack, ID 39). They are checked by decoding
them (ASN.1 UPER):

- **Valid:** logged as JSON in the **Rx** or **Tx** log like any other J2735 message.
- **Invalid** (the message ID is a tolling message but the rest doesn't decode): a **WARN** line in the plugin log,
  for example `Invalid TAM message (ASN.1 UPER decoding failed: ...). Payload: 0025...`, a warning in the V2X Hub
  event log, and the plugin status **Invalid TAM/TUM/TUMack Messages** counts them. Nothing is written to the Rx/Tx
  log for that message.

Other J2735 messages that fail to decode are still reported as errors and counted in **Skipped Messages**.

To receive tolling messages from an RSU, also enable the **Message Receiver** plugin. If they arrive signed (1609.2),
set its `FullSPDUMode` so it unwraps them before they reach this plugin.

## Functionality Testing