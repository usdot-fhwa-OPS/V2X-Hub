# Immediate Forward Plugin Documentation

## Introduction

The Immediate Forward Plugin is responsible for forwarding relevant V2X (Vehicle to everything) messages to RSUs (Road Side Units) for radio broadcast via the immediate forward protocol defined in RSU 4.1 or NTCIP 1218. This plugin can be configured to have multiple independent connections to different RSUs, forwarding different messages and supporting different standards.

## Related Plugins

Most plugins that broadcast messages via RSU are related the the Immediate Forward Plugin.


## Configuration/Deployment

This plugin has several configuration parameters. Below these are listed out as together with descriptions on how to set them.

**ImmediateForwardConfigurations**: This is a JSON array of configurations for unique and independent RSU immediate forward configuration. Below the format for this configuration parameter is described.
> [!TIP]
> Default Immediate Forward protocol port on RSU4.1 is **1516**.
```json
[
            {
                "name": "East Intersection Cohda", /** String Name for RSU **/
                "rsuSpec": "RSU4.1", /** Version of Immediate Forward Protocol to use (RSU4.1 or NTCIP1218) **/
                "address": "127.0.0.1", /** Address of RSU **/
                "port": 1516, /** Default port for Immediate Forward protocol on RSU4.1 **/
                "txMode": "CONT", /** Transmission Mode (CONT or ALT) **/
                "signMessages": false, /** Flag to indicate whether message being forwarded to RSU is already signed**/
                "messages": /** A list of V2X messages to be forwarded to this RSU. Any message types not listed here will not be forwarded to this RSU **/
                [ 
                    { 
                        "tmxType": "SPAT-P", /** TMX message type **/
                        "sendType": "SPAT", /** Message Type **/
                        "psid": "0x8002", /** Message PSID **/
                        "channel": 183 /** (optional) RSU Channel to broadcast from (180 for DSRC and 183 for CV2X)  **/
                    },
                    { "tmxType": "MAP-P", "sendType": "MAP", "psid": "0x8002", "channel": 183 },
                    { "tmxType": "PSM-P", "sendType": "PSM", "psid": "0x27", "channel": 183 }, 
                    { "tmxType": "TIM", "sendType": "TIM", "psid": "0x8003", "channel": 183 },
                    { "tmxType": "TMSG07-P", "sendType": "TMSG07", "psid": "0x8002", "channel": 183 },
                    { "tmxType": "TMSG03-P", "sendType": "TMSG03", "psid": "0xBFEE", "channel": 183 },
                    { "tmxType": "TMSG05-P", "sendType": "TMSG05", "psid": "0x8003", "channel": 183 },
                    { "tmxType": "SSM-P", "sendType": "SSM", "psid": "0x8002", "channel": 183 },
                    { "tmxType": "SDSM", "sendType": "SDSM", "psid": "0x8010", "channel": 183 }
                ]
            },
            /** Setup with V2X Hub HSM message signing and signature verifying. NOTE: This functionality is no longer actively maintained. It is preferred to configure the RSU itself to sign and verify signatures of messages! **/
            {
                "name": "East Intersection Cohda with HSM signing", /** String Name for RSU **/
                "rsuSpec": "RSU4.1", /** Version of Immediate Forward Protocol to use (RSU4.1 or NTCIP1218) **/
                "address": "127.0.0.1", /** Address of RSU **/
                "port": 1516, /** Default port for Immediate Forward protocol on RSU4.1 **/
                "txMode": "CONT", /** Transmission Mode (CONT or ALT) **/
                "signMessages": true, /** Flag to indicate whether message being forwarded to RSU is already signed**/
                "enableHsm": true, /** (Optional : default false) Flag to indicate whether V2X Hub should attempt to sign and verify message signatures via HSM **/ 
                "hsmUrl": "http://<softhsm raspberrypi IP>:3000/v1/scms/", /** (Optional : only read when enableHsm true) URL of HSM API to provide signatures and verify signatures. **/
                "messages": /** A list of V2X messages to be forwarded to this RSU. Any message types not listed here will not be forwarded to this RSU **/
                [ 
                    { 
                        "tmxType": "SPAT-P", /** TMX message type **/
                        "sendType": "SPAT", /** Message Type **/
                        "psid": "0x8002", /** Message PSID **/
                        "channel": 183 /** (optional) RSU Channel to broadcast from (180 for DSRC and 183 for CV2X)  **/
                    },
                    { "tmxType": "MAP-P", "sendType": "MAP", "psid": "0x8002", "channel": 183 },
                    { "tmxType": "PSM-P", "sendType": "PSM", "psid": "0x27", "channel": 183 }, 
                    { "tmxType": "TIM", "sendType": "TIM", "psid": "0x8003", "channel": 183 },
                    { "tmxType": "TMSG07-P", "sendType": "TMSG07", "psid": "0x8002", "channel": 183 },
                    { "tmxType": "TMSG03-P", "sendType": "TMSG03", "psid": "0xBFEE", "channel": 183 },
                    { "tmxType": "TMSG05-P", "sendType": "TMSG05", "psid": "0x8003", "channel": 183 },
                    { "tmxType": "SSM-P", "sendType": "SSM", "psid": "0x8002", "channel": 183 },
                    { "tmxType": "SDSM", "sendType": "SDSM", "psid": "0x8010", "channel": 183 }
                ]
            },
            
        {
            "name": "Yunnex",
            "rsuSpec": "NTCIP1218",
            "address": "192.168.55.123",
            "port": 161,
            "snmpAuth": { /* (optional) SNMP V3 Auth and Encrypt */
                "user": "rsuRwUser",
                "securityLevel": "authPriv",
                "community": "public",
                "authProtocol": "SHA-512",
                "authPassPhrase": "Password",
                "privacyProtocol": "AES-256",
                "privacyPassPhrase": "Password"
            },
            "txMode": "CONT",
            "signMessages": false,
            "messages": [
            {
                "tmxType": "SPAT-P",
                "sendType": "SPAT",
                "psid": "0x8002",
                "channel": 183
            },
            {
                "tmxType": "MAP-P",
                "sendType": "MAP",
                "psid": "0x8002",
                "channel": 183
            }
            ]
        }
        ]    
```





### Messages

All V2X Messages

## Functionality Testing

