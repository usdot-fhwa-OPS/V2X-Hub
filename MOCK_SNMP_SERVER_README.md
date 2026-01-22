# Mock SNMP RSU Server

A Python-based mock SNMP server that simulates an RSU (Roadside Unit) device for testing the RSUHealthMonitorPlugin.

## Features

- ✅ SNMPv3 support with authentication (authPriv)
- ✅ Support for multiple authentication protocols (SHA, SHA-256, SHA-512, etc.)
- ✅ Support for multiple privacy protocols (DES, 3DES, AES-128, AES-256)
- ✅ Simulates both NTCIP 1218 and RSU MIB 4.1 OID structures
- ✅ Returns realistic RSU health data
- ✅ Easy configuration via command-line arguments

## Installation

### Requirements

```bash
# Install pysnmp (maintained fork recommended)
pip3 install pysnmp-lextudio

# Or install original pysnmp
pip3 install pysnmp
```

### On Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install python3 python3-pip
pip3 install pysnmp-lextudio
```

## Usage

### Quick Start

```bash
# Start server with default settings (NTCIP 1218)
python3 mock_snmp_rsu_server.py

# Run as root to bind to port 161 (standard SNMP port)
sudo python3 mock_snmp_rsu_server.py
```

### Common Options

```bash
# Use RSU MIB 4.1 instead of NTCIP 1218
python3 mock_snmp_rsu_server.py --mib RSU41

# Listen on specific IP address
python3 mock_snmp_rsu_server.py --ip 192.168.55.20 --port 161

# Use non-privileged port (doesn't require sudo)
python3 mock_snmp_rsu_server.py --port 1610

# Custom credentials
python3 mock_snmp_rsu_server.py --user myuser --auth-pass mypass --priv-pass mypass
```

### Full Command-Line Options

```
usage: mock_snmp_rsu_server.py [-h] [--ip IP] [--port PORT] 
                                [--mib {NTCIP1218,RSU41}]
                                [--user USER] [--auth-pass AUTH_PASS]
                                [--priv-pass PRIV_PASS]

options:
  -h, --help            show this help message and exit
  --ip IP               IP address to bind to (default: 0.0.0.0)
  --port PORT           UDP port to listen on (default: 161)
  --mib {NTCIP1218,RSU41}
                        MIB version to simulate (default: NTCIP1218)
  --user USER           SNMPv3 username (default: authOnlyUser)
  --auth-pass AUTH_PASS
                        SNMPv3 authentication passphrase
  --priv-pass PRIV_PASS
                        SNMPv3 privacy passphrase
```

## Configuration

### Default Configuration

The server uses these default settings matching your RSUHealthMonitorPlugin configuration:

```json
{
  "ip": "192.168.55.20",
  "port": 161,
  "user": "authOnlyUser",
  "authProtocol": "SHA-512",
  "authPassPhrase": "dummy123",
  "privacyProtocol": "AES-256",
  "privacyPassPhrase": "dummy123",
  "securityLevel": "authPriv",
  "rsuMibVersion": "NTCIP1218"
}
```

### Supported OIDs

#### NTCIP 1218 (Default)

| OID | Name | Example Value |
|-----|------|---------------|
| 1.3.6.1.4.1.1206.4.2.18.13.4.0 | rsuID | RSU-TEST-001 |
| 1.3.6.1.4.1.1206.4.2.18.13.1.0 | rsuMibVersion | NTCIP1218 v01.38 |
| 1.3.6.1.4.1.1206.4.2.18.13.2.0 | rsuFirmwareVersion | v2.5.1-mock |
| 1.3.6.1.4.1.1206.4.2.18.1.2.1.2.0 | rsuRadioDesc | Cohda MK5 OBU |
| 1.3.6.1.4.1.1206.4.2.18.6.5.0 | rsuGnssOutputString | GPS NMEA string |
| 1.3.6.1.4.1.1206.4.2.18.16.1.0 | rsuChanStatus | 0 (bothOp) |
| 1.3.6.1.4.1.1206.4.2.18.16.2.0 | rsuMode | 3 (operate) |

#### RSU MIB 4.1 (Alternative)

| OID | Name | Example Value |
|-----|------|---------------|
| 1.0.15628.4.1.17.4.0 | rsuID | RSU-4.1-TEST-001 |
| 1.0.15628.4.1.17.1.0 | rsuMibVersion | rsuMIB 4.1 rev201812060000Z |
| 1.0.15628.4.1.17.2.0 | rsuFirmwareVersion | v4.1.2-mock |
| 1.0.15628.4.1.17.5.0 | rsuManufacturer | Mock RSU Inc. |
| 1.0.15628.4.1.99.0 | rsuMode | 3 (operate) |
| 1.0.15628.4.1.19.1.0 | rsuChanStatus | 0 (bothOp) |

## Testing with RSUHealthMonitorPlugin

### 1. Update RSUHealthMonitorPlugin Configuration

Update your V2X Hub RSUHealthMonitorPlugin manifest.json:

```json
{
  "key": "RSUConfigurationList",
  "default": "{ \"rsuConfigs\": [ { \"event\": \"test event\", \"rsu\": { \"ip\": \"127.0.0.1\", \"port\": 1610 }, \"snmp\": { \"user\": \"authOnlyUser\", \"authProtocol\": \"SHA-512\", \"authPassPhrase\": \"dummy123\", \"privacyProtocol\": \"AES-256\", \"privacyPassPhrase\": \"dummy123\", \"rsuMibVersion\": \"NTCIP1218\", \"securityLevel\": \"authPriv\" } } ] }"
}
```

### 2. Start the Mock Server

```bash
# Use unprivileged port for testing
python3 mock_snmp_rsu_server.py --port 1610

# Or for production testing on standard port (requires sudo)
sudo python3 mock_snmp_rsu_server.py --ip 192.168.55.20 --port 161
```

### 3. Start RSUHealthMonitorPlugin

The plugin will connect to the mock server and query RSU status.

### 4. Verify Connection

You should see log output like:

```
🚀 Mock RSU SNMP Server - NTCIP 1218 Mode
📡 Listening on 0.0.0.0:1610
👤 User: authOnlyUser
🔐 Auth: SHA-512
🔒 Privacy: AES-256
✅ Server is ready and waiting for SNMP requests...
```

## Testing with snmpwalk/snmpget

You can also test the server manually:

```bash
# Install SNMP tools
sudo apt-get install snmp

# Query all OIDs
snmpwalk -v3 -l authPriv -u authOnlyUser \
  -a SHA-512 -A dummy123 \
  -x AES256 -X dummy123 \
  localhost:1610 1.3.6.1.4.1.1206.4.2.18

# Query specific OID (RSU ID)
snmpget -v3 -l authPriv -u authOnlyUser \
  -a SHA-512 -A dummy123 \
  -x AES256 -X dummy123 \
  localhost:1610 1.3.6.1.4.1.1206.4.2.18.13.4.0
```

## Troubleshooting

### Port Permission Denied

If you get a permission error on port 161:

```bash
# Option 1: Use sudo
sudo python3 mock_snmp_rsu_server.py

# Option 2: Use unprivileged port
python3 mock_snmp_rsu_server.py --port 1610
```

### Port Already in Use

If port 161 is already in use by net-snmp:

```bash
# Stop existing SNMP daemon
sudo systemctl stop snmpd

# Or use different port
python3 mock_snmp_rsu_server.py --port 1610
```

### pysnmp Import Errors

If you see import errors:

```bash
# Try the maintained fork
pip3 install --upgrade pysnmp-lextudio

# Or reinstall
pip3 uninstall pysnmp
pip3 install pysnmp-lextudio
```

## Customization

### Adding Custom OID Values

Edit the `NTCIP1218_OIDS` or `RSU41_OIDS` dictionaries in the script:

```python
NTCIP1218_OIDS = {
    "1.3.6.1.4.1.1206.4.2.18.13.4.0": ("rsuID", "MY-CUSTOM-RSU-ID"),
    # ... add more OIDs
}
```

### Simulating Different RSU States

Modify the status values to simulate different conditions:

```python
# Simulate radio failure
"1.3.6.1.4.1.1206.4.2.18.16.1.0": ("rsuChanStatus", "3"),  # noneOp

# Simulate standby mode
"1.3.6.1.4.1.1206.4.2.18.16.2.0": ("rsuMode", "1"),  # standby
```

## License

This is a testing tool for V2X Hub development.

## References

- [NTCIP 1218 Standard](https://www.ntcip.org/)
- [RSU MIB 4.1 Specification](https://github.com/certificationoperatingcouncil/COC_TestSpecs)
- [PySNMP Documentation](https://pysnmp.readthedocs.io/)
