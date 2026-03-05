# V2X-Hub Network and Security Validation - InSpec Profile

This InSpec profile validates the network configuration, security settings, and operational status of a V2X-Hub Docker deployment. It converts the functionality of the original bash validation script (`validate_network.sh`) into a structured, maintainable InSpec test suite.

## Overview

The profile validates:
- Docker Compose configuration and service status
- Network connectivity and isolation
- Database security and connectivity
- WebSocket reverse proxy configuration
- Environment variable configuration
- Port exposure and security
- Container health status

## Profile Structure

```
v2xhub-validation/
├── inspec.yml                    # Profile metadata and inputs
├── README.md                     # This documentation
├── controls/                     # Test control files
│   ├── docker_compose_spec.rb    # Docker Compose validation
│   ├── services_spec.rb          # Service status checks
│   ├── network_spec.rb           # Network connectivity & isolation
│   ├── database_spec.rb          # Database security & connectivity
│   ├── websocket_spec.rb         # WebSocket proxy configuration
│   ├── environment_spec.rb       # Environment variables
│   ├── ports_spec.rb             # Port exposure validation
│   └── health_spec.rb            # Container health checks
└── libraries/
    └── v2xhub_helper.rb          # Helper methods and utilities
```

## Prerequisites

1. **InSpec Installation**: Install InSpec on your system
   ```bash
   # Via gem
   gem install inspec
   
   # Via package manager (Ubuntu/Debian)
   wget https://packages.chef.io/files/stable/inspec/5.22.3/ubuntu/20.04/inspec_5.22.3-1_amd64.deb
   sudo dpkg -i inspec_5.22.3-1_amd64.deb
   ```

2. **Docker and Docker Compose**: Ensure Docker and Docker Compose are installed and accessible

3. **Running V2X-Hub**: The V2X-Hub services should be running via Docker Compose
   ```bash
   cd configuration
   docker compose up -d
   ```

## Usage

### Basic Execution

Run all tests from the configuration directory:
```bash
cd configuration
inspec exec inspec/v2xhub-validation
```

### Run Specific Control Files

Execute specific test categories:
```bash
# Network and security tests only
inspec exec inspec/v2xhub-validation --controls=/network_spec.rb
inspec exec inspec/v2xhub-validation --controls=/database_spec.rb

# Service status tests
inspec exec inspec/v2xhub-validation --controls=/services_spec.rb
```

### Generate Reports

Create detailed reports in various formats:
```bash
# HTML report
inspec exec inspec/v2xhub-validation --reporter html:v2xhub_validation_report.html

# JSON report for CI/CD integration
inspec exec inspec/v2xhub-validation --reporter json:v2xhub_validation_report.json

# CLI and JSON combined
inspec exec inspec/v2xhub-validation --reporter cli json:report.json
```

### Custom Input Values

Override default configuration values:
```bash
inspec exec inspec/v2xhub-validation --input mysql_port=3307 websocket_port=19761
```

## Test Categories

### 1. Docker Compose Configuration (`docker_compose_spec.rb`)
- Validates Docker Compose accessibility
- Checks compose file structure and required services
- Verifies network and secrets configuration

### 2. Service Status (`services_spec.rb`)
- Ensures core services (db, php, v2xhub) are running
- Validates service dependencies and restart policies
- Checks container states

### 3. Network Connectivity and Isolation (`network_spec.rb`)
- Tests inter-service connectivity (v2xhub↔db, php↔db, php↔v2xhub)
- Validates network isolation (internal vs external networks)
- Verifies network subnet configurations

### 4. Database Security (`database_spec.rb`)
- Ensures database is not exposed externally
- Validates database health and connectivity
- Checks credential management via Docker secrets
- Verifies database initialization

### 5. WebSocket Proxy Configuration (`websocket_spec.rb`)
- Validates Apache WebSocket reverse proxy setup
- Checks required Apache modules
- Verifies WebSocket environment variables
- Ensures WebSocket port is not directly exposed

### 6. Environment Variables (`environment_spec.rb`)
- Validates required MySQL connection variables
- Checks V2XHub core configuration
- Verifies environment variable consistency

### 7. Port Exposure and Security (`ports_spec.rb`)
- Ensures core ports (80, 443, 8686) are properly exposed
- Validates that sensitive ports (3306, 19760) are NOT exposed
- Checks port configuration in Docker Compose

### 8. Container Health (`health_spec.rb`)
- Monitors container health status
- Validates health check configuration
- Checks resource limits and system resources
- Verifies service dependencies

## Security Focus

The profile emphasizes security validation:

### Critical Security Controls (Impact 1.0)
- Database port (3306) must NOT be exposed externally
- WebSocket port (19760) must NOT be exposed externally
- Core services must be running and accessible
- MySQL credentials must be managed via Docker secrets

### High Priority Controls (Impact 0.8-0.9)
- Network isolation between internal and external networks
- Service connectivity validation
- WebSocket reverse proxy configuration
- Environment variable security

## Configuration Inputs

The profile accepts the following input parameters:

| Input | Default | Description |
|-------|---------|-------------|
| `mysql_host` | `db` | MySQL database hostname |
| `mysql_port` | `3306` | MySQL database port |
| `mysql_database` | `IVP` | MySQL database name |
| `mysql_user` | `IVP` | MySQL database user |
| `websocket_port` | `19760` | V2XHub WebSocket port |
| `v2x_port` | `8686` | V2X Messages port |
| `web_http_port` | `80` | Web UI HTTP port |
| `web_https_port` | `443` | Web UI HTTPS port |

## CI/CD Integration

### Example GitHub Actions Workflow
```yaml
name: V2X-Hub Validation
on: [push, pull_request]

jobs:
  validate:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install InSpec
        run: |
          wget https://packages.chef.io/files/stable/inspec/5.22.3/ubuntu/20.04/inspec_5.22.3-1_amd64.deb
          sudo dpkg -i inspec_5.22.3-1_amd64.deb
      - name: Start V2X-Hub
        run: |
          cd configuration
          docker compose up -d
          sleep 30  # Wait for services to start
      - name: Run Validation
        run: |
          cd configuration
          inspec exec inspec/v2xhub-validation --reporter json:validation_report.json
      - name: Upload Results
        uses: actions/upload-artifact@v3
        with:
          name: validation-report
          path: configuration/validation_report.json
```

## Troubleshooting

### Common Issues

1. **Docker Compose Not Found**
   ```
   Error: Docker Compose is not accessible
   Solution: Ensure Docker Compose is installed and the service is running
   ```

2. **Services Not Running**
   ```
   Error: Service 'db' is not running
   Solution: Check docker compose logs and ensure services started properly
   ```

3. **Network Connectivity Failures**
   ```
   Error: V2XHub cannot reach database
   Solution: Verify Docker networks exist and containers are on correct networks
   ```

4. **Permission Issues**
   ```
   Error: Cannot execute docker commands
   Solution: Ensure user has Docker permissions or run with sudo
   ```

### Debug Commands

```bash
# Check Docker Compose status
docker compose ps

# View service logs
docker compose logs db
docker compose logs v2xhub
docker compose logs php

# Inspect networks
docker network ls
docker network inspect configuration_v2xhub_internal

# Check container connectivity
docker compose exec v2xhub ping db
```

## Migration from Bash Script

This InSpec profile replaces the original `validate_network.sh` script with the following advantages:

- **Structured Testing**: Organized into logical control groups
- **Better Reporting**: Rich HTML/JSON reports with detailed results
- **CI/CD Ready**: Easy integration with automated pipelines
- **Maintainable**: Clear separation of concerns and reusable components
- **Cross-Platform**: Works on Linux, macOS, and Windows
- **Compliance Ready**: Built-in compliance reporting capabilities

## Contributing

To add new tests or modify existing ones:

1. Add new controls to appropriate files in `controls/`
2. Update helper methods in `libraries/v2xhub_helper.rb` if needed
3. Update input parameters in `inspec.yml` if required
4. Test changes with `inspec check inspec/v2xhub-validation`
5. Document new functionality in this README

## Support

For issues related to:
- **InSpec Profile**: Check InSpec documentation or file issues in the V2X-Hub repository
- **V2X-Hub Deployment**: Refer to V2X-Hub documentation and deployment guides
- **Docker Issues**: Check Docker and Docker Compose documentation
