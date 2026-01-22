# Mock NATS Registration Server

This mock server responds to TelematicBridge RSU registration requests with "ok" to simulate a successful registration.

## Installation

Install the required Python package:

```bash
pip install nats-py
```

## Usage

### Basic Usage (local NATS server)

```bash
python3 mock_nats_registration_server.py
```

### Connect to Custom NATS Server

```bash
python3 mock_nats_registration_server.py --nats-url nats://192.168.1.100:4222
```

## How It Works

1. **Connects** to the NATS server at the specified URL
2. **Subscribes** to the wildcard topic: `unit.*.register.rsu.config`
3. **Listens** for registration requests from TelematicBridge plugins
4. **Responds** with "ok" to each request, marking the registration as successful
5. **Logs** all registration activity with timestamps and unit information

## Expected Behavior

When TelematicBridge sends a registration request:
- The mock server receives the JSON registration data
- It parses the unit ID and RSU configuration
- It responds with "ok" (required for successful registration)
- The TelematicBridge will then proceed with normal operation

## Example Output

```
============================================================
Mock NATS Registration Server for TelematicBridge
============================================================
✓ Connected to NATS server at nats://localhost:4222
✓ Subscribed to: unit.*.register.rsu.config
  Waiting for registration requests...
  Press Ctrl+C to stop

[2026-01-21 10:30:45] Received registration request:
  Subject: unit.test-unit-123.register.rsu.config
  Reply-To: _INBOX.abc123xyz
  Data: {"unit_id":"test-unit-123", ...}
  ✓ Responded with: ok
  Total registered units: 1
```

## Stopping the Server

Press `Ctrl+C` to gracefully shutdown the server.

## Troubleshooting

### Connection Failed
- Verify NATS server is running
- Check the NATS URL and port are correct
- Ensure no firewall is blocking the connection

### No Registration Requests
- Verify TelematicBridge is configured with correct NATS URL
- Check that `INFRASTRUCTURE_ID` environment variable is set
- Review TelematicBridge logs for connection errors

### Registration Not Working
- Ensure this mock server responds with "ok" (case-sensitive)
- Verify the topic pattern matches: `unit.*.register.rsu.config`
- Check TelematicBridge logs for "Received registered reply" messages
