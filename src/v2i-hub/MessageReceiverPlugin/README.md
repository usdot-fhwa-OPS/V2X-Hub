# MessageReceiverPlugin Documentation

## Introduction

The `MessageReceiverPlugin` is the V2X Hub plugin that listens for incoming V2X messages, normally from an RSU and forwards them to down stream plugins. It currently supports receiving J2735 messages as UPER encoded HEX over UDP and forwarding decoded J2735 messages as well as receiving 1609.2 messages and fowarding these as RawSpdu messages.

## Configuration/Deployment

This plugin has several configuration parameters. Below these are listed out as together with descriptions on how to set them.

**Port**: Port for the incoming message network connection (UDP)

**RouteJ2735**: Set the flag to route/broadcast a received J2735 message to TMX Core.

**FullSPDUMode**: If enabled, the MessageReceiver Plugin will expect messages with the 1609.2 header and forward these as RawSpdu messages.

## Design

## Messages

**J2735Message**: All J2735 V2X messages described in the SAE J2735 specification

**RawSpdMessage**: The original undecoded bytes representing a IEEE 1609.2 message which will included an SAE J2735 message

`RawSpdu` messages are published with the `IvpMsgFlags_RouteDSRC` flag and with DSRC metadata attached, using the PSID unwrapped from the SPDU and the default channel of 183. This allows the [Immediate Forward Plugin](../ImmediateForwardPlugin/README.md) to pick them up and re-broadcast the original SPDU bytes unmodified.

> [!IMPORTANT]
> When forwarding SPDUs for re-broadcast, set **RouteJ2735** to `false`. Otherwise, in `FullSPDUMode`, each received message is routed twice: once as the unsecured J2735 message and once as the `RawSpdu`, which results in the Immediate Forward Plugin broadcasting it twice.

## Functionality Testing 

### Test Normal Mode

1) Start V2X Hub and enable Message Receiver Plugin
2) Configure plugin to FullSPDUMode=false
3) Run `src/v2ihub/MessageReceiver/scripts/broadcast_bsm.py`
4) Confirm Message Receiver State tab in UI is receiving BSMs using the message counts

### Test FullSPDUMode Mode

1) Start V2X Hub and enable Message Receiver Plugin
2) Configure plugin to FullSPDUMode=true
3) Run `src/v2ihub/MessageReceiver/scripts/sendSpdu.py 50`
4) Confirm Message Receiver State tab in UI is receiving SPDU using the SPDU processed counts




