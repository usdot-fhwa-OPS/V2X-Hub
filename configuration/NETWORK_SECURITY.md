# V2X-Hub Network Security Implementation Summary

## Overview
This document summarizes the network security enhancements implemented for V2X-Hub to improve security through proper network segmentation and isolation, and port management.

## Changes Made

### Docker Compose Configuration (`docker-compose.yml`)
- **Removed**: `network_mode: host` from all services
- **Added**: Three dedicated networks:
  - **Web Network** (`v2xhub_web_external`): Handles external web traffic
  - **Application Network** (`v2xhub_app_external`): Manages application logic and communication
  - **Data Network** (`v2xhub_data_internal`): Isolated database operations with no external access  
- **Port Management**: Only necessary ports exposed to host
- **Switched**: to container service names (e.g., `db` instead of `127.0.0.1`)

## Migration Considerations

### Breaking Changes
- Services can no longer communicate via localhost/127.0.0.1
  - In general plugin configurations that used `127.0.0.1` to point to v2xhub should use `0.0.0.0`
- Inbound port mappings must be **explicitly** configured
  - Port ranges can be implemented to allow flexibility in v2xhub plugin configurations (see MessageReceiverPlugin)
- Network-dependent configurations require updates

### Compatibility
- External services connecting to V2X-Hub may need configuration updates
- Monitoring tools may need to be reconfigured for new network architecture

## Security Improvements

### Before (Insecure) - Highlevel Network Security Diagram
```
┌─────────────────────────────────────────┐
│              Host Network               │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐    │
│  │   DB    │ │   PHP   │ │ V2XHub  │    │
│  │ :3306   │ │ :80/443 │ │ :19760* │    │
│  │ EXPOSED │ │ EXPOSED │ │ EXPOSED │    │
│  └─────────┘ └─────────┘ └─────────┘    │
│  ⚠️ALL PORTS DIRECTLY ACCESSIBLE⚠️     │
└─────────────────────────────────────────┘
```

### After (Secure) - Highlevel Network Security Diagram
```
┌────────────────────────────────────────────────────────┐
│                 Host Network                           │
│   ┌─────────┐        ┌───────────────────────┐         │
│   │ :80/443 │        │ :19760 + other Plugins│         │
│   └────┬────┘        └────────┬──────────────┘         │
└────────┼──────────────────────┼────────────────────────┘
         │                      │  
         │                      │     
┌────────┼──────────────────────┼────────────────────────┐
│        │         Web Network  │                        │
│   ┌────▼────┐                 │                        │
│   │   php   │                 │                        │
│   └─────────┘                 │                        │
│ ⚠️ The php container cannot reach v2xhub directly. ⚠️ │
│     Traffic is sent via TLS WebSocket from the browser │
│                               │                        │
└───────────────────────────────┼────────────────────────┘
                                │
┌───────────────────────────────┼────────────────────────┐
│                 App Network   │                        │
│   ┌───────────────────────────▼───────────────────┐    │
│   │                  v2xhub                       │    │
│   │                                               │    │
│   │ Command Plugin                                │    │
│   │ :19760             # TLS WebSocket            │    │
│   │                                               │    │
│   │ **Plugins with Inbound Ports                  │    │
│   │                                               │    │
│   │ Message Reciever Plugin                       │    │
│   │ :26780-26800/udp   # Message Receiver         │    │
│   │                                               │    │
│   │ SPaT Plugin                                   │    │
│   │ :6053/udp          # TSC Sends UDP            │    │
│   │ :5050/udp          # SNMP Polling             │    │
│   │                                               │    │
│   │ TIM Plugin                                    │    │
│   │ :10000/tcp         # TIM Web Service Port     │    │
│   │                                               │    │
│   │ CARMA Cloud Plugin                            │    │
│   │ :22222/tcp         # CARMA Cloud Plugin       │    │
│   │                                               │    │
│   │ CDASim Adapter Plugin (simulation mode)       │    │
│   │ :6767/udp          # CDASim Registration      │    │
│   │ :7575/udp          # Time sync                │    │
│   │ :5757/udp          # Simulated V2X out        │    │
│   │                                               │    │
│   │ MUST Sensor Driver Plugin                     │    │
│   │ :4545/udp          # MUST Detection Receiver  │    │
│   │                                               │    │
│   └───────────────────────────┬───────────────────┘    │
└───────────────────────────────┼────────────────────────┘
                                │
┌───────────────────────────────┼────────────────────────┐
│                 Data Network  │                        │
│   ┌──────────┐          ┌─────▼───┐                    │
│   │ db :3306 │◄─────────│ v2xhub  │                    │
│   └──────────┘          └─────────┘                    │
│                 INTERNAL ONLY                          │
└────────────────────────────────────────────────────────┘

```

## Port Exposure Summary

### Core Services
| Service | Port | Exposure | Purpose | Security Level |
|---------|------|----------|---------|----------------|
| **MySQL** | 3306 | Internal Only | Database | ✅ Secure |
| **PHP** | 80 | External | HTTP Redirect | ✅ Secure |
| **PHP** | 443 | External | HTTPS Web UI | ✅ Secure |
| **V2X-Hub** | 19760 | External | V2X UI TLS WebSocket | ✅ Secure |

### Plugin Inbound Ports
| Plugin | Port(s) | Protocol | Exposure | Purpose |
|--------|---------|----------|----------|---------|
| CommandPlugin | 19760 | TCP | External | Command WebSocket |
| MessageReceiverPlugin | 26780-26800 | UDP | External | External Message Reception |
| SPaTPlugin | 6053 5050 | UDP | External | TSC sends and SNMP polling |
| TIMPlugin | 10000 | TCP | External | TIM WebServicePort |
| CARMACloudPlugin | 22222 | TCP | External | CARMA Cloud Integration |
| CDASimAdapter | 6767 7575 5757 | UDP | External | CDASim registration, Time sync, Simulated V2X out |
| MUSTSensorDriverPlugin | 4545 | UDP | External | MUST detection receiver |
| Port Drayage WebService | 8090 | TCP | External | Port Drayage REST API/Web UI |

> **⚠️ SECURITY WARNING**: Plugin inbound ports increase attack surface. These ports remain open at the Docker level even when plugins are disabled in the V2X-Hub UI. 

## Verify Network Connectivity
```bash
# Test internal database connectivity
docker exec -it v2xhub getent hosts db

# Check exposed ports
docker port php 
docker port v2xhub 
docker db v2xhub 

# Inspect Networks
docker network ls
docker network inspect configuration_v2xhub_web_external
docker network inspect configuration_v2xhub_app_external
docker network inspect configuration_v2xhub_data_internal
```

## Security Benefits Achieved

### ✅ **Defense in Depth**
- Multiple network layers provide redundant security controls
- Compromise of one tier doesn't automatically expose others

### ✅ **Principle of Least Privilege**
- Services only have access to networks they require
    - PHP: only `v2xhub_web_external`
    - V2XHub: `v2xhub_app_external` + `v2xhub_data_internal`
    - DB: only `v2xhub_data_internal`
- Database has no external connectivity (`internal: true`)
- Explicit port mappings minimize exposure
- No unnecessary external access

### ✅ **Network Segmentation and Isolation**
- Database traffic isolated on the internal data network (`internal: true`) prevents external access
- Container-to-container communication is encrypted and controlled
- No shared host network vulnerabilities (no `network_mode: host`)
- Clear separation between internal and external networks
- Inter-service communication protected

### ✅ **Reduced Attack Surface**
- Elimination of host network mode reduces exposure
- Explicit port mapping instead of blanket host access
- Database completely isolated from external access
- Service-specific network assignments

### ✅ **Improved Monitoring and Control**
- Network traffic can be monitored per segment
- Easier to implement network-level security policies
- Clear separation of concerns for security auditing

### ✅ **Service Isolation**
- Each service runs in controlled network environment
- Prevents lateral movement in case of compromise
- Docker DNS-based service discovery
