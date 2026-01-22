#!/usr/bin/env python3
"""
Mock NATS Registration Server for TelematicBridge Testing

This script simulates the NATS registration service that responds to
RSU registration requests from the TelematicBridge plugin.

Usage:
    python3 mock_nats_registration_server.py [--nats-url NATS_URL]

Requirements:
    pip install nats-py
"""

import asyncio
import argparse
import json
import sys
from datetime import datetime
from nats.aio.client import Client as NATS


class MockRegistrationServer:
    def __init__(self, nats_url="nats://localhost:4222"):
        self.nats_url = nats_url
        self.nc = NATS()
        self.registered_units = {}
        
    async def connect(self):
        """Connect to NATS server"""
        try:
            await self.nc.connect(servers=[self.nats_url])
            print(f"✓ Connected to NATS server at {self.nats_url}")
            return True
        except Exception as e:
            print(f"✗ Failed to connect to NATS server: {e}")
            return False
    
    async def handle_registration_request(self, msg):
        """Handle RSU registration requests"""
        subject = msg.subject
        reply = msg.reply
        data = msg.data.decode()
        
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(f"\n[{timestamp}] Received registration request:")
        print(f"  Subject: {subject}")
        print(f"  Reply-To: {reply}")
        print(f"  Data: {data[:200]}..." if len(data) > 200 else f"  Data: {data}")
        
        try:
            # Parse the incoming registration data
            registration_data = json.loads(data)
            
            # Extract unit ID if present
            unit_id = "unknown"
            if isinstance(registration_data, dict):
                unit_id = registration_data.get("unit_id", "unknown")
            elif isinstance(registration_data, list) and len(registration_data) > 0:
                # For RSU registration arrays
                unit_id = f"unit_with_{len(registration_data)}_rsus"
            
            # Store registration info
            self.registered_units[unit_id] = {
                "timestamp": timestamp,
                "data": registration_data
            }
            
            # Send "ok" response as expected by TelematicRsuUnit
            response = "ok"
            await self.nc.publish(reply, response.encode())
            print(f"  ✓ Responded with: {response}")
            print(f"  Total registered units: {len(self.registered_units)}")
            
        except json.JSONDecodeError as e:
            print(f"  ✗ Failed to parse JSON: {e}")
            error_response = "error: invalid JSON"
            await self.nc.publish(reply, error_response.encode())
        except Exception as e:
            print(f"  ✗ Error processing request: {e}")
            error_response = "error: processing failed"
            await self.nc.publish(reply, error_response.encode())
    
    async def subscribe_to_registrations(self):
        """Subscribe to RSU registration requests"""
        # Subscribe to the wildcard topic to catch all unit registrations
        # Pattern: unit.*.register.rsu.config
        registration_topic = "unit.*.register.rsu.config"
        
        try:
            await self.nc.subscribe(registration_topic, cb=self.handle_registration_request)
            print(f"✓ Subscribed to: {registration_topic}")
            print(f"  Waiting for registration requests...")
            print(f"  Press Ctrl+C to stop\n")
        except Exception as e:
            print(f"✗ Failed to subscribe: {e}")
            return False
        
        return True
    
    async def run(self):
        """Main run loop"""
        if not await self.connect():
            return False
        
        if not await self.subscribe_to_registrations():
            await self.nc.close()
            return False
        
        # Keep running until interrupted
        try:
            while True:
                await asyncio.sleep(1)
        except KeyboardInterrupt:
            print("\n\nShutting down...")
            await self.nc.close()
            print("✓ Disconnected from NATS")
            print(f"Total units registered: {len(self.registered_units)}")
            return True


def main():
    parser = argparse.ArgumentParser(
        description='Mock NATS Registration Server for TelematicBridge',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Connect to local NATS server
  python3 mock_nats_registration_server.py
  
  # Connect to custom NATS server
  python3 mock_nats_registration_server.py --nats-url nats://192.168.1.100:4222
        """
    )
    parser.add_argument(
        '--nats-url',
        default='nats://localhost:4222',
        help='NATS server URL (default: nats://localhost:4222)'
    )
    
    args = parser.parse_args()
    
    print("=" * 60)
    print("Mock NATS Registration Server for TelematicBridge")
    print("=" * 60)
    
    server = MockRegistrationServer(nats_url=args.nats_url)
    
    try:
        asyncio.run(server.run())
    except Exception as e:
        print(f"✗ Server error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
