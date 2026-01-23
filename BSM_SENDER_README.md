# BSM Message Sender for V2X Hub

This Python script sends BSM (Basic Safety Message) packets to the V2X Hub MessageReceiver Plugin via UDP.

## Overview

The script sends J2735-encoded BSM messages to the MessageReceiver plugin, which listens on UDP port 26789 by default. BSMs are typically transmitted at 10 Hz (10 times per second) to simulate vehicle-to-infrastructure communication.

## Requirements

- Python 3.x
- Network connectivity to V2X Hub (local or remote)
- V2X Hub with MessageReceiver Plugin running

## Usage

### Basic Usage

Send BSM messages continuously at 10 Hz:
```bash
python3 send_bsm_to_message_receiver.py
```

### Send Limited Number of Messages

Send only 100 BSM messages:
```bash
python3 send_bsm_to_message_receiver.py --count 100
```

### Change Transmission Rate

Send at 5 Hz instead of 10 Hz:
```bash
python3 send_bsm_to_message_receiver.py --rate 5
```

### Use Different BSM Sample

The script includes 3 pre-configured BSM samples:
```bash
python3 send_bsm_to_message_receiver.py --bsm 2
```

### Send to Remote Host

Send to a V2X Hub running on a different machine:
```bash
python3 send_bsm_to_message_receiver.py --host 192.168.1.100 --port 26789
```

### Use Custom BSM Hex Data

Send your own J2735-encoded BSM hex string:
```bash
python3 send_bsm_to_message_receiver.py --hex "0014A1D9A3B50C4000..."
```

### Verbose Output

Show detailed output for every message sent:
```bash
python3 send_bsm_to_message_receiver.py --verbose
```

## Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `--host` | Target IP address | 127.0.0.1 |
| `--port` | Target UDP port | 26789 |
| `--rate` | Transmission rate in Hz | 10 |
| `--count` | Number of messages to send | Continuous |
| `--bsm` | Select BSM sample (1-3) | 1 |
| `--hex` | Custom BSM hex string | None |
| `-v, --verbose` | Enable verbose output | False |

## Examples

### Example 1: Test with 10 Messages
```bash
python3 send_bsm_to_message_receiver.py --count 10 --verbose
```

### Example 2: High-Rate Transmission
```bash
python3 send_bsm_to_message_receiver.py --rate 20 --count 200
```

### Example 3: Remote Testing
```bash
python3 send_bsm_to_message_receiver.py --host 10.0.0.5 --port 26789 --rate 10
```

## Stopping the Script

To stop continuous transmission, press `Ctrl+C`. The script will display the total number of messages sent before exiting.

## Verifying Messages

To verify that V2X Hub is receiving the BSM messages:

1. Check the V2X Hub logs
2. Monitor the MessageReceiver plugin status
3. Use the V2X Hub web interface to view incoming messages

## Troubleshooting

### Messages Not Being Received

1. Verify V2X Hub is running: `docker ps` or check service status
2. Verify MessageReceiver plugin is enabled in V2X Hub
3. Check firewall rules allow UDP traffic on port 26789
4. Verify the host IP and port are correct

### Connection Issues

If sending to a remote host:
- Ensure network connectivity: `ping <host>`
- Check firewall rules on both machines
- Verify the V2X Hub is listening on the correct interface

## Message Format

The BSM messages are J2735-encoded binary data sent as UDP packets. Each BSM contains:
- Vehicle ID (temporary)
- Position (latitude, longitude, elevation)
- Motion (speed, heading, acceleration)
- Vehicle size
- Additional safety information

## Original Script

This script is based on the original `sendBsms.py` located at:
`src/v2i-hub/MessageReceiverPlugin/scripts/sendBsms.py`

## License

This script is part of the V2X Hub project. See the main project LICENSE file for details.
