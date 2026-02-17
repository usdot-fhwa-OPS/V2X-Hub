# V2X-Hub Network Security Configuration

## Overview

This document describes the network security enhancements implemented for V2X-Hub to improve security by isolating internal services and reducing the attack surface through proper network segmentation.

## Security Improvements

### Previous Configuration Issues
- All services used `network_mode: host` exposing ALL container ports directly on the host
- No network isolation between services
- Database accessible from external network
- Internal service communication exposed unnecessarily

### New Architecture
- **Private Internal Network**: `v2xhub_internal` for database and inter-service communication
- **External Network**: `v2xhub_external` for services requiring external access
- **Port Isolation**: Only necessary ports are exposed to the host
- **Service Discovery**: Services communicate using Docker DNS names

## Network Architecture

### Core Services (Always Deployed)
```
┌──────────────────────────────────────────────────────────────┐
│                        Host Network                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐   │
│  │   Port 80   │  │  Port 443   │  │    Port 8686        │   │
│  │   (HTTP)    │  │  (HTTPS)    │  │   (V2X Messages)    │   │
│  └─────────────┘  └─────────────┘  └─────────────────────┘   │
│         │                 │                    │             │
└─────────┼─────────────────┼────────────────────┼─────────────┘
          │                 │                    │
┌─────────┼─────────────────┼────────────────────┼─────────────┐
│         │    v2xhub_external network           │             │
│         │           (172.21.0.0/16)            │             │
│  ┌──────▼──────┐                         ┌─────▼──────┐      │
│  │     PHP     │                         │   V2XHub   │      │
│  │  Container  │                         │ Container  │      │
│  └──────┬──────┘                         └─────┬──────┘      │
│         │                                      │             │
└─────────┼──────────────────────────────────────┼─────────────┘
          │                                      │
┌─────────┼──────────────────────────────────────┼─────────────┐
│         │     v2xhub_internal network          │             │
│         │          (172.20.0.0/16)             │             │
│         │          internal: true              │             │
│  ┌──────▼──────┐                        ┌──────▼──────┐      │
│  │     PHP     │◄──────────────────────►│    V2XHub   │      │
│  │  Container  │                        │  Container  │      │
│  └──────┬──────┘                        └──────┬──────┘      │
│         │                                      │             │
│         │              ┌───────────────────────┘             │
│         │              │                                     │
│  ┌──────▼──────────────▼──────┐                              │
│  │       MySQL Database       │                              │
│  │        Container           │                              │
│  │     (No external ports)    │                              │
│  └────────────────────────────┘                              │
└──────────────────────────────────────────────────────────────┘
```

### Plugin Inbound Ports (Profile-Based Deployment)
```
┌─────────────────────────────────────────────────────────────┐
│                        Host Network                         │
│ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────────────┐ │
│ │Port 26789│ │Port 22222│ │Port 44444│ │ Simulation Ports │ │
│ │MessageRcv│ │CARMA Cld │ │ERV Cloud │ │ 6767,7575,5757,  │ │
│ │(v2x-ess) │ │(carma-cl)│ │(erv-cl)  │ │ 7576 (simulation)│ │
│ └──────────┘ └──────────┘ └──────────┘ └──────────────────┘ │
│      │            │            │              │             │
│      │            │            │              │             │
│      │            │            │              │             │
└──────┼────────────┼────────────┼──────────────┼─────────────┘
       │            │            │              │
┌──────┼────────────┼────────────┼──────────────┼─────────────┐
│      │    v2xhub_external network             │             │
│      │           (172.21.0.0/16)              │             │
│      │                                        │             │
│ ┌────▼────────────────────────────────────────▼──────┐      │
│ │              V2XHub Container                      │      │
│ │                                                    │      │
│ │  ┌─────────────────┐  ┌─────────────────────────┐  │      │
│ │  │ MessageReceiver │  │    Plugin Manager       │  │      │
│ │  │    Plugin       │  │                         │  │      │
│ │  │   Port 26789    │  │  ┌─────────────────────┐│  │      │
│ │  └─────────────────┘  │  │  CARMACloudPlugin   ││  │      │
│ │                       │  │    Port 22222       ││  │      │
│ │  ┌─────────────────┐  │  └─────────────────────┘│  │      │
│ │  │ERVCloudForward  │  │                         │  │      │
│ │  │    Plugin       │  │  ┌─────────────────────┐│  │      │
│ │  │   Port 44444    │  │  │   CDASimAdapter     ││  │      │
│ │  └─────────────────┘  │  │ Ports 6767,7575,etc ││  │      │
│ │                       │  └─────────────────────┘│  │      │
│ │                       └─────────────────────────┘  │      │
│ └────────────────────────────────────────────────────┘      │
│                                                             │
└─────────────────────────────────────────────────────────────┘

Profile Usage:
• --profile v2x-essential    → Exposes port 26789 (MessageReceiver)
• --profile carma-cloud      → Exposes port 22222 (CARMA Cloud)
• --profile erv-cloud        → Exposes port 44444 (ERV Cloud)
• --profile simulation       → Exposes ports 6767,7575,5757,7576
• --profile port_drayage     → Exposes port 8090 (separate service)
```

## Port Configuration

### Core Exposed Ports (Always External Access)

| Service | Port | Protocol | Purpose | Security Notes |
|---------|------|----------|---------|----------------|
| **PHP/Apache** | 80 | HTTP | Web UI (redirects to HTTPS) | Redirects to 443 for security |
| **PHP/Apache** | 443 | HTTPS | Secure Web UI | Required for admin interface |
| **V2X-Hub** | 8686 | TCP | V2X Message Communication | Required for RSU/OBU communication |

### Plugin Inbound Ports (Profile-Based)

| Plugin | Port | Protocol | Purpose | Profile |
|--------|------|----------|---------|---------|
| **MessageReceiverPlugin** | 26789 | TCP | V2X Radio Messages | `v2x-essential` |
| **CARMACloudPlugin** | 22222 | HTTP | CARMA Cloud TCM Messages | `carma-cloud` |
| **ERVCloudForwardingPlugin** | 44444 | HTTP | Emergency Vehicle BSMs | `erv-cloud` |
| **CDASimAdapter** | 6767 | TCP | Simulation Registration | `simulation` |
| **CDASimAdapter** | 7575 | TCP | Time Synchronization | `simulation` |
| **CDASimAdapter** | 5757 | TCP | Simulation V2X Messages | `simulation` |
| **CDASimAdapter** | 7576 | TCP | Simulation Interaction | `simulation` |
| **Port Drayage** | 8090 | HTTP | REST API | `port_drayage` |

#### ⚠️ Security Warning: Plugin Port Management

**IMPORTANT**: When using Docker port mappings, ports remain **open at the OS level** even when plugins are disabled in the V2X-Hub UI. This creates potential security vulnerabilities:

- **Disabled Plugin Risk**: Port 22222 stays open even if CARMACloudPlugin is disabled
- **Information Disclosure**: Port scanners can detect open ports regardless of plugin state
- **Attack Surface**: Unnecessary ports increase potential entry points

**Recommendation**: Only expose ports for plugins you actively use by selecting appropriate Docker Compose profiles.

### Internal Ports (No External Access)

| Service | Port | Protocol | Purpose | Network |
|---------|------|----------|---------|---------|
| **MySQL** | 3306 | TCP | Database Connection | `v2xhub_internal` only |

## Environment Variables

### Port Configuration Variables
```bash
# Web UI Ports
WEB_HTTP_PORT=80
WEB_HTTPS_PORT=443

# V2X Communication Ports
V2X_PORT=8686

# Simulation Ports (only used when SIMULATION_MODE=TRUE)
SIMULATION_REGISTRATION_PORT=6767
TIME_SYNC_PORT=7575
SIM_V2X_PORT=5757
SIM_INTERACTION_PORT=7576

# Port Drayage Service Port (only used with port_drayage profile)
PORT_DRAYAGE_PORT=8090
```

### Database Connection
- **Previous**: `127.0.0.1:3306` or `localhost:3306`
- **New**: `db:3306` (using Docker service name)

## Security Benefits

### 1. **Reduced Attack Surface**
- Database is completely isolated from external access
- Only necessary ports are exposed to the host
- Internal service communication is protected

### 2. **Network Segmentation**
- Clear separation between internal and external networks
- Database traffic isolated on internal network
- External services can't directly access internal resources

### 3. **Service Isolation**
- Each service runs in its own network namespace
- Inter-service communication controlled by network policies
- Prevents lateral movement in case of compromise

### 4. **Principle of Least Privilege**
- Services only have access to networks they need
- Database has no external network access
- Simulation ports only exposed when needed

## Migration Notes

### Breaking Changes
1. **Database Connection**: Services now connect to `db:3306` instead of `127.0.0.1:3306`
2. **Port Binding**: Ports are now explicitly bound instead of using host networking
3. **Service Discovery**: Services use Docker DNS names for internal communication

### Compatibility
- External API endpoints remain the same
- Web UI still accessible on ports 80/443
- V2X communication still on port 8686
- All existing functionality preserved

## Deployment Instructions

### Docker Compose Profile-Based Deployment

#### 1. **Minimal Secure Deployment (Core Only)**
```bash
cd configuration/
docker compose up -d
```
**Exposed Ports**: 80, 443, 8686
**Use Case**: Basic V2X-Hub without external integrations

#### 2. **Essential V2X Deployment (Recommended)**
```bash
cd configuration/
docker compose --profile v2x-essential up -d
```
**Exposed Ports**: 80, 443, 8686, 26789
**Use Case**: Full V2X functionality with radio message reception

#### 3. **CARMA Cloud Integration**
```bash
cd configuration/
docker compose --profile v2x-essential --profile carma-cloud up -d
```
**Exposed Ports**: 80, 443, 8686, 26789, 22222
**Use Case**: V2X with CARMA Cloud traffic control integration

#### 4. **Emergency Vehicle Integration**
```bash
cd configuration/
docker compose --profile v2x-essential --profile erv-cloud up -d
```
**Exposed Ports**: 80, 443, 8686, 26789, 44444
**Use Case**: V2X with emergency vehicle cloud forwarding

#### 5. **Development/Simulation Environment**
```bash
cd configuration/
docker compose --profile v2x-essential --profile simulation up -d
```
**Exposed Ports**: 80, 443, 8686, 26789, 6767, 7575, 5757, 7576
**Use Case**: Development and testing with CDASim integration

#### 6. **Full Integration Deployment**
```bash
cd configuration/
docker compose --profile v2x-essential --profile carma-cloud --profile erv-cloud --profile port_drayage up -d
```
**Exposed Ports**: 80, 443, 8686, 26789, 22222, 44444, 8090
**Use Case**: Production deployment with all integrations

### Verification Commands

#### Check Service Status
```bash
docker compose ps
```

#### Verify Network Connectivity
```bash
# Test internal database connectivity
docker compose exec v2xhub ping db
docker compose exec php ping db

# Check exposed ports
docker compose port php 443
docker compose port v2xhub 8686
```

#### Validate Plugin Ports
```bash
# Check which ports are actually exposed
docker compose port v2xhub

# Test plugin port connectivity (if enabled)
nc -zv localhost 26789  # MessageReceiver
nc -zv localhost 22222  # CARMA Cloud (if profile active)
nc -zv localhost 44444  # ERV Cloud (if profile active)
```
## Troubleshooting

### Common Issues

#### 1. **Database Connection Errors**
```bash
# Check if database is accessible from v2xhub container
docker compose exec v2xhub ping db

# Check database logs
docker compose logs db
```

#### 2. **Port Binding Conflicts**
```bash
# Check if ports are already in use
netstat -tulpn | grep :80
netstat -tulpn | grep :443
netstat -tulpn | grep :8686

```

#### 3. **Network Connectivity Issues**
```bash
# Inspect networks
docker network ls
docker network inspect configuration_v2xhub_internal
docker network inspect configuration_v2xhub_external

# Check service network assignments
docker compose exec v2xhub ip route
```

### Rollback Procedure
If issues occur, you can temporarily rollback by:
1. Restore the original `docker-compose.yml` with `network_mode: host`
2. Update `container/service.sh` to use `127.0.0.1:3306`
3. Redeploy services

## Testing Checklist

- [ ] Database connectivity from all services
- [ ] Web UI accessible on HTTPS
- [ ] V2X message communication working
- [ ] Simulation mode ports (if enabled)
- [ ] Port drayage service (if profile active)
- [ ] SSL certificate validation
- [ ] Service startup and health checks
- [ ] Log file accessibility
- [ ] Volume mount functionality

## Conclusion

This network security enhancement significantly improves the security posture of V2X-Hub by:
- Isolating the database from external access
- Reducing the number of exposed ports
- Implementing proper network segmentation
- Following Docker security best practices

The changes maintain full functionality while providing a much more secure deployment architecture.
