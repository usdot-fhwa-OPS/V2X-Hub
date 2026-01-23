#!/usr/bin/env python3
"""
Send NATS Request to Query Available Topics

This script sends a NATS request to query available topics from a specific RSU unit
in the V2X Hub TelematicBridge plugin and prints the response.

Usage:
    python3 send_nats_request_available_topics.py [options]

Requirements:
    pip install nats-py
"""

import asyncio
import argparse
import json
import sys
from datetime import datetime
from nats.aio.client import Client as NATS


async def send_nats_request(nats_url, unit_id, timeout=5.0):
    """
    Send NATS request to query available topics
    
    Args:
        nats_url: NATS server URL
        unit_id: Unit ID to query (e.g., "rsu_1234")
        timeout: Request timeout in seconds
    
    Returns:
        Response data as string, or None if failed
    """
    nc = NATS()
    
    try:
        # Connect to NATS server
        print(f"Connecting to NATS server: {nats_url}")
        await nc.connect(servers=[nats_url])
        print(f"✓ Connected to NATS server\n")
        
        # Construct the topic
        topic = f"unit.{unit_id}.topic.rsu.available_topics"
        print(f"Sending request to topic: {topic}")
        print(f"Timeout: {timeout} seconds")
        print("-" * 60)
        
        # Send request with empty payload (request doesn't need data)
        try:
            response = await nc.request(topic, b"", timeout=timeout)
            
            # Get response data
            response_data = response.data.decode('utf-8')
            
            print("\n✓ Response received!")
            print("=" * 60)
            print("RAW RESPONSE:")
            print("=" * 60)
            print(response_data)
            print()
            
            # Try to parse and pretty-print JSON
            try:
                response_json = json.loads(response_data)
                print("=" * 60)
                print("FORMATTED JSON RESPONSE:")
                print("=" * 60)
                print(json.dumps(response_json, indent=2))
                print()
                
                # Display summary information
                print("=" * 60)
                print("SUMMARY:")
                print("=" * 60)
                
                if isinstance(response_json, dict):
                    if "unitId" in response_json:
                        print(f"Unit ID: {response_json.get('unitId', 'N/A')}")
                    
                    if "timestamp" in response_json:
                        timestamp = response_json.get('timestamp', 'N/A')
                        print(f"Timestamp: {timestamp}")
                    
                    if "rsuTopics" in response_json and isinstance(response_json["rsuTopics"], list):
                        print(f"\nNumber of RSUs: {len(response_json['rsuTopics'])}")
                        
                        for idx, rsu_data in enumerate(response_json["rsuTopics"], 1):
                            if "rsuEndpoint" in rsu_data:
                                endpoint = rsu_data["rsuEndpoint"]
                                rsu_ip = endpoint.get("ip", "unknown")
                                rsu_port = endpoint.get("port", "unknown")
                                print(f"\n  RSU #{idx}: {rsu_ip}:{rsu_port}")
                            
                            if "topics" in rsu_data and isinstance(rsu_data["topics"], list):
                                topics = rsu_data["topics"]
                                print(f"    Available topics: {len(topics)}")
                                for topic_data in topics:
                                    if isinstance(topic_data, dict):
                                        topic_name = topic_data.get("name", "unknown")
                                        is_selected = topic_data.get("selected", False)
                                        status = "✓" if is_selected else " "
                                        print(f"      [{status}] {topic_name}")
                                    elif isinstance(topic_data, str):
                                        print(f"        - {topic_data}")
                
                print()
                
            except json.JSONDecodeError:
                print("Note: Response is not valid JSON format")
                print()
            
            return response_data
            
        except asyncio.TimeoutError:
            print(f"\n✗ Request timed out after {timeout} seconds")
            print("  Possible reasons:")
            print("  - V2X Hub TelematicBridge plugin is not running")
            print("  - Unit ID does not exist or is not registered")
            print("  - NATS server is not properly configured")
            return None
            
    except Exception as e:
        print(f"\n✗ Error: {e}")
        return None
        
    finally:
        # Close connection
        if nc.is_connected:
            await nc.close()
            print("Disconnected from NATS server")


def main():
    parser = argparse.ArgumentParser(
        description='Send NATS request to query RSU available topics',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Query unit rsu_1234 on local NATS server
  python3 send_nats_request_available_topics.py
  
  # Query specific unit with custom timeout
  python3 send_nats_request_available_topics.py --unit-id my_unit --timeout 10
  
  # Connect to remote NATS server
  python3 send_nats_request_available_topics.py --nats-url nats://192.168.1.100:4222 --unit-id rsu_1234
        """
    )
    
    parser.add_argument(
        '--nats-url',
        default='nats://localhost:4222',
        help='NATS server URL (default: nats://localhost:4222)'
    )
    
    parser.add_argument(
        '--unit-id',
        default='rsu_1234',
        help='Unit ID to query (default: rsu_1234)'
    )
    
    parser.add_argument(
        '--timeout',
        type=float,
        default=5.0,
        help='Request timeout in seconds (default: 5.0)'
    )
    
    args = parser.parse_args()
    
    print("=" * 60)
    print("NATS Request: Query RSU Available Topics")
    print("=" * 60)
    print()
    
    # Run async request
    response = asyncio.run(send_nats_request(args.nats_url, args.unit_id, args.timeout))
    
    if response:
        return 0
    else:
        return 1


if __name__ == '__main__':
    sys.exit(main())
