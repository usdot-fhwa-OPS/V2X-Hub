# V2X-Hub Network Security Implementation Summary

## Overview
This document summarizes the network security enhancements implemented for V2X-Hub to improve security through proper network isolation and port management.

## Changes Made

### 1. Docker Compose Configuration (`docker-compose.yml`)
- **Removed**: `network_mode: host` from all services
- **Added**: Two dedicated networks:
  - `v2xhub_internal` (172.20.0.0/16) - Internal communication only
  - `v2xhub_external` (172.21.0.0/16) - External access for required services
- **Database**: Completely isolated on internal network only
- **Port Management**: Only necessary ports exposed to host

### 2. Environment Configuration (`.env`)
- **Added**: Port configuration variables for easy management
- **Added**: Network security documentation comments

### 3. Service Configuration (`container/service.sh`)
- **Updated**: Database connection to use service name `db:3306` instead of `127.0.0.1:3306`

### 4. Docker Compose Profiles
- **Implemented**: Native Docker Compose profiles for conditional service deployment
- **Features**: 
  - Simulation ports only exposed when `--profile simulation` is used
  - Port drayage service only exposed when `--profile port_drayage` is used
  - No additional scripts needed - uses Docker Compose native functionality

### 5. Documentation
- **Created**: `NETWORK_SECURITY.md` - Comprehensive security documentation
- **Created**: `DEPLOYMENT_SECURITY_SUMMARY.md` - Implementation summary

## Security Improvements

### Before (Insecure) - Highlevel Network Security Diagram
```
┌─────────────────────────────────────────┐
│              Host Network               │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐    │
│  │   DB    │ │   PHP   │ │ V2XHub  │    │
│  │ :3306   │ │ :80/443 │ │ :8686   │    │
│  │ EXPOSED │ │ EXPOSED │ │ EXPOSED │    │
│  └─────────┘ └─────────┘ └─────────┘    │
│  ALL PORTS DIRECTLY ACCESSIBLE          │
└─────────────────────────────────────────┘
```

### After (Secure) - Highlevel Network Security Diagram
```
┌─────────────────────────────────────────┐
│              Host Network               │
│     ┌─────────┐ ┌─────────────────┐     │
│     │ :80/443 │ │     :8686       │     │
│     └─────────┘ └─────────────────┘     │
└─────────┼─────────────────┼─────────────┘
          │                 │
┌─────────┼─────────────────┼─────────────┐
│         │  External Net   │             │
│  ┌──────▼──────┐   ┌──────▼──────┐      │
│  │     PHP     │   │   V2XHub    │      │
│  └──────┬──────┘   └──────┬──────┘      │
└─────────┼─────────────────┼─────────────┘
          │                 │
┌─────────┼─────────────────┼─────────────┐
│         │  Internal Net   │             │
│  ┌──────▼──────┐   ┌──────▼──────┐      │
│  │     PHP     │   │   V2XHub    │      │
│  └──────┬──────┘   └─────────────┘      │
│         │                               │
│  ┌──────▼──────┐                        │
│  │ DB :3306    │                        │
│  │ INTERNAL    │                        │
│  │ ONLY        │                        │
│  └─────────────┘                        │
└─────────────────────────────────────────┘
```

### After (Secure) - Detailed Network Security Diagram
```
┌──────────────────────────────────────────────────────────────┐
│                        Host Network                          │
│ ┌─────────┐ ┌─────────┐ ┌──────────┐ ┌─────────────────────┐ │
│ │ :80/443 │ │ :8686   │ │Plugin    │ │   Conditional       │ │
│ │ (Core)  │ │ (Core)  │ │Ports     │ │   Profile Ports     │ │
│ │         │ │         │ │26789     │ │ 22222,44444,6767... │ │
│ └─────────┘ └─────────┘ │22222     │ └─────────────────────┘ │
│                         │44444     │                         │
│                         │(Profile) │                         │
│                         └──────────┘                         │
└─────────┼─────────┼─────────┼─────────────────┼──────────────┘
          │         │         │                 │
┌─────────┼─────────┼─────────┼─────────────────┼──────────────┐
│         │  External Network │                 │              │
│  ┌──────▼──────┐   ┌────────▼─────────────────▼──────┐       │
│  │     PHP     │   │         V2XHub Container        │       │
│  │  Container  │   │                                 │       │
│  └──────┬──────┘   │  ┌─────────────────────────────┐│       │
│         │          │  │      Plugin Manager         ││       │
│         │          │  │                             ││       │
│         │          │  │ ┌─────────┐ ┌─────────────┐ ││       │
│         │          │  │ │MessageRx│ │CARMA Cloud  │ ││       │
│         │          │  │ │:26789   │ │Plugin :22222│ ││       │
│         │          │  │ └─────────┘ └─────────────┘ ││       │
│         │          │  │                             ││       │
│         │          │  │ ┌─────────┐ ┌─────────────┐ ││       │
│         │          │  │ │ERV Cloud│ │CDASimAdapter│ ││       │
│         │          │  │ │:44444   │ │:6767,7575...│ ││       │
│         │          │  │ └─────────┘ └─────────────┘ ││       │
│         │          │  └─────────────────────────────┘│       │
│         │          └─────────────────┬───────────────┘       │
└─────────┼─────────────────────────────┼──────────────────────┘
          │                           │
┌─────────┼─────────────────────────────┼──────────────────────┐
│         │     Internal Network        │                      │
│  ┌──────▼──────┐             ┌────────▼─────────────────┐    │
│  │     PHP     │◄───────────►│      V2XHub Container    │    │
│  │  Container  │             │                          │    │
│  └──────┬──────┘             └──────────────────────────┘    │
│         │                                                    │
│  ┌──────▼──────┐                                             │
│  │ DB :3306    │                                             │
│  │ INTERNAL    │                                             │
│  │ ONLY        │                                             │
│  └─────────────┘                                             │
└──────────────────────────────────────────────────────────────┘
```
## Port Exposure Summary

### Core Services
| Service | Port | Exposure | Purpose | Security Level |
|---------|------|----------|---------|----------------|
| **MySQL** | 3306 | Internal Only | Database | ✅ Secure |
| **PHP** | 80 | External | HTTP Redirect | ✅ Secure |
| **PHP** | 443 | External | HTTPS Web UI | ✅ Secure |
| **V2X-Hub** | 8686 | External | V2X Messages | ✅ Required |
| **V2X-Hub** | 6767 | Conditional | Simulation | ✅ Only when needed |
| **V2X-Hub** | 7575 | Conditional | Time Sync | ✅ Only when needed |
| **V2X-Hub** | 5757 | Conditional | Sim V2X | ✅ Only when needed |
| **V2X-Hub** | 7576 | Conditional | Sim Interaction | ✅ Only when needed |
| **Port Drayage** | 8090 | Conditional | REST API | ✅ Only when needed |

### Plugin Inbound Ports
| Plugin | Port | Exposure | Purpose | Recommended Profile |
|--------|------|----------|---------|-------------------|
| **MessageReceiverPlugin** | 26789 | Conditional | External Message Reception | `message_receiver` |
| **CARMACloudPlugin** | 22222 | Conditional | CARMA Cloud Integration | `carma_cloud` |
| **ERVCloudForwardingPlugin** | 44444 | Conditional | Emergency Response Vehicle | `erv_cloud` |
| **CDASimAdapter** | Various | Conditional | CARMA/CDA Simulation | `simulation` |

> **⚠️ SECURITY WARNING**: Plugin inbound ports increase attack surface. These ports remain open at the Docker level even when plugins are disabled in the V2X-Hub UI. Use Docker Compose profiles to control inbound port exposure at the container level. Only enable these profiles when the specific plugin functionality is actively required.

## Deployment Instructions

### 1. Standard Deployment (Secure - No Simulation Ports)
```bash
cd configuration/
docker compose up -d
```

### 2. Simulation Mode Deployment (Exposes Simulation Ports)
```bash
cd configuration/
docker compose --profile simulation up -d
```

### 3. Port Drayage Deployment
```bash
cd configuration/
docker compose --profile port_drayage up -d
```

### 4. Combined Deployment (Simulation + Port Drayage)
```bash
cd configuration/
docker compose --profile simulation --profile port_drayage up -d
```

### 5. Plugin-Specific Deployments

#### MessageReceiverPlugin
```bash
cd configuration/
docker compose --profile message_receiver up -d
```

#### CARMACloudPlugin
```bash
cd configuration/
docker compose --profile carma_cloud up -d
```

#### ERVCloudForwardingPlugin
```bash
cd configuration/
docker compose --profile erv_cloud up -d
```

#### Multiple Plugin Deployment
```bash
cd configuration/
docker compose --profile simulation --profile carma_cloud --profile erv_cloud up -d
```

> **⚠️ SECURITY NOTICE**: Plugin inbound ports (26789, 22222, 44444) increase attack surface. Use Docker Compose profiles to control inbound port exposure at the container level. Only enable these profiles when the specific plugin functionality is actively required.

## Validation Commands

### Check Service Status
```bash
docker compose ps
```

### Verify Network Connectivity
```bash
# Test internal database connectivity
docker compose exec v2xhub ping db
docker compose exec php ping db

# Check exposed ports
docker compose port php 443
docker compose port v2xhub 8686
```

### Inspect Networks
```bash
docker network ls
docker network inspect configuration_v2xhub_internal
docker network inspect configuration_v2xhub_external
```

## Security Benefits Achieved

### ✅ Reduced Attack Surface
- Database completely isolated from external access
- Only 3 ports exposed by default (80, 443, 8686)
- Simulation ports only exposed when needed
- Port drayage API only exposed when profile active

### ✅ Network Segmentation
- Clear separation between internal and external networks
- Database traffic isolated on internal network
- Inter-service communication protected

### ✅ Service Isolation
- Each service runs in controlled network environment
- Prevents lateral movement in case of compromise
- Docker DNS-based service discovery

### ✅ Principle of Least Privilege
- Services only have access to networks they need
- Conditional port exposure based on functionality
- No unnecessary external access

## Monitoring and Maintenance

### Regular Security Checks
```bash
# Check for unauthorized port exposure
netstat -tulpn | grep -E ':(80|443|8686|6767|7575|5757|7576|8090)'

# Monitor network connections
docker compose logs --tail=100 -f

# Verify network isolation
docker compose exec db netstat -tulpn
```

## Rollback Plan

If issues occur, rollback is possible by:

1. **Restore original configuration**:
   ```bash
   git checkout HEAD -- configuration/docker-compose.yml
   git checkout HEAD -- container/service.sh
   ```

2. **Remove network configuration**:
   ```bash
   docker compose down
   docker network prune -f
   ```

3. **Redeploy with original settings**:
   ```bash
   docker compose up -d
   ```

## Conclusion

The network security implementation successfully:
- ✅ Isolates the database from external access
- ✅ Reduces exposed ports 
- ✅ Implements proper network segmentation
- ✅ Maintains full V2X-Hub functionality
- ✅ Provides conditional port exposure
- ✅ Follows Docker security best practices
- ✅ Includes comprehensive documentation
- ✅ Provides easy deployment and management tools

This implementation significantly improves V2X-Hub's security posture while maintaining operational excellence.
