# V2X-Hub Infrastructure Security Tests - InSpec Profile

This InSpec profile tests the network configuration, security settings, and operational status of a V2X-Hub Docker deployment.

## Prerequisites

### Cinc Auditor (InSpec) Installation

Install the OSS distribution of InSpec, Cinc Auditor. 

#### Option 1: Install via script (recommended for Linux)
```bash
curl -L https://omnitruck.cinc.sh/install.sh | sudo bash -s -- -P cinc-auditor -v 6
sudo ln -sf /usr/bin/cinc-auditor /usr/local/bin/inspec
inspec version
```

#### Option 2: Install via RubyGems
```bash
gem install cinc-auditor-bin --clear-sources \
  -s https://rubygems.cinc.sh -s https://rubygems.org

sudo ln -sf /usr/bin/cinc-auditor /usr/local/bin/inspec
inspec version
```

#### License Considerations
> Chef InSpec requires enterprise, governments, and organizations to purchase a commercial license which is why CINC Auditor was choosen.  CINC is available for use under Apache 2.0 license.
- [Chef End User License Agreement (EULA)](https://www.chef.io/end-user-license-agreement)
- [CINC](https://cinc.sh/download/)

## Overview

The profile validates:
- Docker Compose configuration and service status
- Network connectivity and isolation
- Database security and connectivity
- Port exposure and security
- Container health status

## Profile Structure

```
v2xhub-validation/
├── inspec.yml                    # Profile metadata and inputs
├── README.md                     # This documentation
├── controls/                     # Test control files
│   ├── database_spec.rb          # Database security & connectivity
│   ├── health_spec.rb            # Container health checks
│   ├── network_spec.rb           # Network connectivity & isolation
│   └── ports_spec.rb             # Port exposure validation
└── libraries/
    └── v2xhub_helper.rb          # Helper methods and utilities
```

## Prerequisites

1. **InSpec Installation**: Install OSS Distribution of InSpec (Cinc Auditor)  on your system
   ```bash
   # Via gem:
   gem install cinc-auditor-bin --clear-sources -s https://rubygems.cinc.sh -s https://rubygems.org
   sudo ln -s /usr/bin/cinc-auditor /usr/local/bin/inspec

   # Via install script (Ubuntu):
   curl -L https://omnitruck.cinc.sh/install.sh | sudo bash -s -- -P cinc-auditor -v 6
   sudo ln -s /usr/bin/cinc-auditor /usr/local/bin/inspec
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
# Execute tests in specified file only
inspec exec inspec/v2xhub-validation/controls/database_spec.rb
inspec exec inspec/v2xhub-validation/controls/health_spec.rb
inspec exec inspec/v2xhub-validation/controls/network_spec.rb
inspec exec inspec/v2xhub-validation/controls/ports_spec.rb


# Execute group of tests specified by tag
inspec exec inspec/v2xhub-validation --tags=network
inspec exec inspec/v2xhub-validation --tags=database

# Execute specified test controls only
inspec exec inspec/v2xhub-validation --controls='v2xhub-network-01'
inspec exec inspec/v2xhub-validation --controls='v2xhub-ports-01'
```

### Generate Reports

Create detailed reports in various formats:
```bash
# HTML report
inspec exec inspec/v2xhub-validation --reporter html:v2xhub_inspec_test_report.html

# JSON report for CI/CD integration
inspec exec inspec/v2xhub-validation --reporter json:v2xhub_inspec_test_report.json

# CLI and JSON combined
inspec exec inspec/v2xhub-validation --reporter cli json:v2xhub_inspec_test_report.json
```

### Custom Input Values

Override default configuration values:
```bash
inspec exec inspec/v2xhub-validation --input mysql_port=3307 websocket_port=19761
```

## Test Categories

### Database Security (`database_spec.rb`)
- Ensures database is not exposed externally
- Validates database health and connectivity
- Checks credential management via Docker secrets
- Verifies database initialization

### Container Health (`health_spec.rb`)
- Monitors container health status
- Validates health check configuration
- Checks resource limits and system resources
- Checks compose file structure and required services
- Validates service dependencies and restart policies

### Network Connectivity and Isolation (`network_spec.rb`)
- Tests inter-service connectivity (v2xhub↔db, php↔db, php↔v2xhub)
- Validates network isolation (internal vs external networks)
- Verifies network subnet configurations

### Port Exposure and Security (`ports_spec.rb`)
- Ensures core ports (80, 443, 8686) are properly exposed
- Validates that sensitive ports (3306, 19760) are NOT exposed
- Checks port configuration in Docker Compose

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
| `v2x_port` | `8686` | V2XHub message bus port |
| `ivp_default_port` | `24601` | V2XHub IVP plugin port |
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
      - name: Install OSS InSpec (CINC Auditor)
        run: |
          curl -L https://omnitruck.cinc.sh/install.sh | sudo bash -s -- -P cinc-auditor -v 6
          sudo ln -sf /usr/bin/cinc-auditor /usr/local/bin/inspec
          inspec version
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

1. **Could Not Fetch Inspec Profile**
   ```
   Error: Could not fetch inspec profile in "inspec/v2xhub-validation".
   Solution: Ensure test command is executed from `configuration` directory
   ```

2. **Docker Compose Not Found**
   ```
   Error: Docker Compose is not accessible
   Solution: Ensure Docker Compose is installed and the service is running
   ```

3. **Services Not Running**
   ```
   Error: Service 'db' is not running
   Solution: Check docker compose logs and ensure services started properly
   ```

4. **Network Connectivity Failures**
   ```
   Error: V2XHub cannot reach database
   Solution: Verify Docker networks exist and containers are on correct networks
   ```

5. **Permission Issues**
   ```
   Error: Cannot execute docker commands
   Solution: Ensure user has Docker permissions or run with sudo
   ```
## Contributing

To add new tests or modify existing ones:

1. Add new controls to appropriate files in `controls/`
2. Update helper methods in `libraries/v2xhub_helper.rb` if needed
3. Update input parameters in `inspec.yml` if required
4. Test changes with `inspec check inspec/v2xhub-validation`
5. Document new functionality in this README