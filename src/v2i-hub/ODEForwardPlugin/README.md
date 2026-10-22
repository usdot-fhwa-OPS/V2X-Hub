# ODE Forward Plugin Documentation

## Introduction
The ODE Forward Plugin is a plugin used to integrate with Joint Program Office [(JPO) Operational Data Environment (ODE)](https://github.com/usdot-jpo-ode/jpo-ode). The ODE application is used by several CV Pilot deployments for processing and storing CV V2X Data. Additionally the ODE application is used as a data source for the [JPO Connected Vehicle (CV) Manager](https://github.com/usdot-jpo-ode/jpo-cvmanager) application, used for continuous validation and management of CV deployements. The ODE Forward Plugin V2X Messages when ever the underlying content or message revisions changes, after first running edge validation on the message. Additionaly the ODE Forwar Plugin, will send information about validation errors encountered at the edge to ODE. This is part of a V2X Hub / CV Manager integration where V2X Hub does some initial message content validation according to [Connected Transporation Interoperaility (CTI) 4501 Connected Intersections Implementation Guide ](https://www.ite.org/pub/?id=76270782-B7E4-7F75-BC72-D5E318B14C9A) and CV Manager does some additional validation and CV deployment management.

## Related Plugins

A list of plugins related to the ODE Forwarding Plugin

### Intersection Validation Plugin

This is the plugin that performances the validation on received V2X messages. It generates both the messages and the validation events to be forwarded to ODE via the ODE Forwarding Plugin

## Configuration/Deployment

This plugin has several configuration parameters. Below these are listed out as together with descriptions on how to set them.

**OdeIp**: This is the IP address of ODE. The ODE Plugin will attempt to send all UDP and Kafka messages to this IP

**KafkaBrokerPort**: This is the port for the kafka broker. The ODE Forward Plugin will attempt to connect to kafka via <OdeIp>:<KafkaBrokerPort>.

**TIMUDPPort**: The port to send TIM UPER UDP messages to.

**SPATUDPPort**: The port to send SPAT UPER UDP messages to.

**MAPUDPPort**: The port to send MAP UPER UDP messages to.

**BMSUDPPort**: The port to send BSM UPER UDP messages to.

> [!NOTE]
> Please see [(JPO) Operational Data Environment (ODE)](https://github.com/usdot-jpo-ode/jpo-ode) documentation for information about ODE ports and the different message formwats and types accepted on each interface.

## Design

## Messages

**TmxJ2735Message**: J2735 V2X Messages to forward to ODE

**CTI4501ValidationMessage**: Validation events that occurred in Intersection Validation Plugin, to forward to ODE
