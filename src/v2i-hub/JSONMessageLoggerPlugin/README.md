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

### Invalid messages

Each J2735 message is decoded (ASN.1 UPER) before it is logged. If decoding fails (the message ID is readable but the
rest is not valid ASN.1), the plugin writes a **WARN** line in the plugin log, for example
`Invalid TAM message (ASN.1 UPER decoding failed: ...). Payload: 0025...`, and a warning in the V2X Hub event log, and
counts it in the plugin status **Invalid `<type>` Messages** (for example **Invalid TAM Messages**). Nothing is written
to the Rx/Tx log for that message. Other errors are reported as before and counted in **Skipped Messages**.

### Tolling messages (TAM, TUM, TUMack)

With the J2735 2024 build (the default), V2X Hub recognizes the tolling messages from SAE J3217: Toll Advertisement
(TAM, ID 37), Toll Usage (TUM, ID 38) and Toll Usage Acknowledgment (TUMack, ID 39). Valid ones are logged as JSON like
any other J2735 message; invalid ones are reported as above. To receive them from an RSU, also enable the
**Message Receiver** plugin. If they arrive signed (1609.2), set its `FullSPDUMode` so it unwraps them first.

## Functionality Testing