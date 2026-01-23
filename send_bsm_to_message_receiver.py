#!/usr/bin/env python3
"""
Send BSM (Basic Safety Message) to V2X Hub MessageReceiver Plugin

This script sends J2735 encoded BSM messages via UDP to the MessageReceiver plugin.
The MessageReceiver plugin listens on port 26789 by default and processes incoming
J2735 messages.

Usage:
    python send_bsm_to_message_receiver.py [options]

Options:
    --host <ip>          Target host IP address (default: 127.0.0.1)
    --port <port>        Target UDP port (default: 26789)
    --rate <hz>          Transmission rate in Hz (default: 10)
    --count <n>          Number of messages to send (default: continuous)
    --bsm <number>       Select BSM sample (1-3, default: 1)
"""

import socket
import binascii
import time
import argparse
import sys

# Sample BSM messages (J2735 encoded in hexadecimal format)
BSM_SAMPLES = {
    1: "001480CF4B950C400022D2666E923D1EA6D4E28957BD55FFFFF001C758FD7E67D07F7FFF8000000002020218E1C1004A40196FBC042210115C030EF1408801021D4074CE7E1848101C5C0806E8E1A50101A84056EE8A1AB4102B840A9ADA21B9010259C08DEE1C1C560FFDDBFC070C0222210018BFCE309623120FFE9BFBB10C8238A0FFDC3F987114241610009BFB7113024780FFAC3F95F13A26800FED93FDD51202C5E0FE17BF9B31202FBAFFFEC87FC011650090019C70808440C83207873800000000001095084081C903447E31C12FC0",
    2: "0014A1D9A3B50C400022D2666E923D1EA6D4E28957BD55FFFFF001C758FD7E67D07F7FFF8000000002020218E1C1004A40196FBC042210115C030EF1408801021D4074CE7E1848101C5C0806E8E1A50101A84056EE8A1AB4102B840A9ADA21B9010259C08DEE1C1C560FFDDBFC070C0222210018BFCE309623120FFE9BFBB10C8238A0FFDC3F987114241610009BFB7113024780FFAC3F95F13A26800FED93FDD51202C5E0FE17BF9B31202FBAFFFEC87FC011650090019C70808440C83207873800000000001095084081C903447E31C12FC0",
    3: "0014B2E8C1D50C400022D2666E923D1EA6D4E28957BD55FFFFF001C758FD7E67D07F7FFF8000000002020218E1C1004A40196FBC042210115C030EF1408801021D4074CE7E1848101C5C0806E8E1A50101A84056EE8A1AB4102B840A9ADA21B9010259C08DEE1C1C560FFDDBFC070C0222210018BFCE309623120FFE9BFBB10C8238A0FFDC3F987114241610009BFB7113024780FFAC3F95F13A26800FED93FDD51202C5E0FE17BF9B31202FBAFFFEC87FC011650090019C70808440C83207873800000000001095084081C903447E31C12FC0"
}


def parse_arguments():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(
        description='Send BSM messages to V2X Hub MessageReceiver Plugin',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    parser.add_argument('--host', type=str, default='127.0.0.1',
                        help='Target host IP address (default: 127.0.0.1)')
    parser.add_argument('--port', type=int, default=26789,
                        help='Target UDP port (default: 26789)')
    parser.add_argument('--rate', type=float, default=10.0,
                        help='Transmission rate in Hz (default: 10)')
    parser.add_argument('--count', type=int, default=None,
                        help='Number of messages to send (default: continuous)')
    parser.add_argument('--bsm', type=int, choices=[1, 2, 3], default=1,
                        help='Select BSM sample 1-3 (default: 1)')
    parser.add_argument('--hex', type=str, default=None,
                        help='Custom BSM hex string to send')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Enable verbose output')
    
    return parser.parse_args()


def send_bsm_messages(host, port, bsm_hex, rate, count=None, verbose=False):
    """
    Send BSM messages via UDP
    
    Args:
        host: Target IP address
        port: Target UDP port
        bsm_hex: Hexadecimal string of the BSM message
        rate: Transmission rate in Hz
        count: Number of messages to send (None for continuous)
        verbose: Enable verbose output
    """
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # Convert hex string to bytes
    try:
        bsm_bytes = binascii.unhexlify(bsm_hex)
    except binascii.Error as e:
        print(f"Error: Invalid hexadecimal string: {e}", file=sys.stderr)
        return False
    
    # Calculate sleep time between messages
    sleep_time = 1.0 / rate if rate > 0 else 0.1
    
    print(f"Sending BSM messages to {host}:{port}")
    print(f"Rate: {rate} Hz (one message every {sleep_time:.3f} seconds)")
    print(f"Message size: {len(bsm_bytes)} bytes")
    if count:
        print(f"Total messages: {count}")
    else:
        print("Mode: Continuous (press Ctrl+C to stop)")
    print("-" * 60)
    
    sent_count = 0
    
    try:
        while True:
            # Send the BSM message
            sock.sendto(bsm_bytes, (host, port))
            sent_count += 1
            
            if verbose or sent_count % 100 == 0:
                print(f"Sent {sent_count} BSM message(s)")
            
            # Check if we've reached the count limit
            if count and sent_count >= count:
                print(f"\nCompleted: Sent {sent_count} BSM message(s)")
                break
            
            # Sleep to maintain the desired rate
            time.sleep(sleep_time)
            
    except KeyboardInterrupt:
        print(f"\n\nInterrupted: Sent {sent_count} BSM message(s)")
    except Exception as e:
        print(f"\nError: {e}", file=sys.stderr)
        return False
    finally:
        sock.close()
    
    return True


def main():
    """Main function"""
    args = parse_arguments()
    
    # Determine which BSM hex string to use
    if args.hex:
        bsm_hex = args.hex
        print(f"Using custom BSM hex string")
    else:
        bsm_hex = BSM_SAMPLES[args.bsm]
        print(f"Using BSM sample #{args.bsm}")
    
    # Send the messages
    success = send_bsm_messages(
        host=args.host,
        port=args.port,
        bsm_hex=bsm_hex,
        rate=args.rate,
        count=args.count,
        verbose=args.verbose
    )
    
    return 0 if success else 1


if __name__ == '__main__':
    sys.exit(main())
