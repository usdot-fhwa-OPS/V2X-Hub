#!/usr/bin/env python3
"""
Send NATS Request to Select Topics for an RSU

This script sends a NATS request to select/configure topics for a specific RSU
in the V2X Hub TelematicBridge plugin.

Note: The system uses RSU IP address as the key for topic selection. The port
field is included in requests for compatibility and metadata purposes but is
not used for matching.

Usage:
    python3 send_nats_request_select_topics.py [options]
    python3 send_nats_request_select_topics.py --rsu-ip 127.0.0.1  --topics J2735_BSM_MessageReceiver


Requirements:
    pip install nats-py
"""

import asyncio
import argparse
import json
import sys
from datetime import datetime
from nats.aio.client import Client as NATS


async def send_select_topics_request(nats_url, unit_id, rsu_ip, rsu_port, topics, timeout=5.0):
    """
    Send NATS request to select topics for a specific RSU
    
    Args:
        nats_url: NATS server URL
        unit_id: Unit ID (e.g., "Unit001")
        rsu_ip: RSU IP address
        rsu_port: RSU port number
        topics: List of topic names to select
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
        
        # Construct the NATS topic for selecting RSU topics
        nats_topic = f"unit.{unit_id}.topic.rsu.selected_topics"
        print(f"Sending request to NATS topic: {nats_topic}")
        print(f"Target RSU: {rsu_ip}:{rsu_port}")
        print(f"Topics to select: {topics}")
        print(f"Timeout: {timeout} seconds")
        print("-" * 60)
        
        # Construct request payload with new format
        # Note: Port is included for compatibility but system only uses IP as key
        request_payload = {
            "rsuTopics": [
                {
                    "rsuEndpoint": {
                        "ip": rsu_ip,
                        "port": rsu_port if rsu_port else 0
                    },
                    "topics": [
                        {
                            "name": topic,
                            "selected": True
                        } for topic in topics
                    ]
                }
            ],
            "timestamp": str(int(datetime.now().timestamp() * 1000)),
            "unitId": unit_id
        }
        
        request_json = json.dumps(request_payload, indent=2)
        print("\nREQUEST PAYLOAD:")
        print("=" * 60)
        print(request_json)
        print("=" * 60)
        
        # Send request
        try:
            response = await nc.request(nats_topic, request_json.encode('utf-8'), timeout=timeout)
            
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
                
                # Display summary
                print("=" * 60)
                print("SUMMARY:")
                print("=" * 60)
                
                if isinstance(response_json, dict):
                    # Check if response has the new format with rsuTopics
                    if "rsuTopics" in response_json:
                        unit_id_resp = response_json.get("unitId", "")
                        timestamp = response_json.get("timestamp", "")
                        
                        print(f"Unit ID: {unit_id_resp}")
                        print(f"Timestamp: {timestamp}")
                        
                        rsu_topics = response_json.get("rsuTopics", [])
                        if rsu_topics:
                            print("\nSelected topics confirmed:")
                            for rsu_topic in rsu_topics:
                                endpoint = rsu_topic.get("rsuEndpoint", {})
                                rsu_ip = endpoint.get("ip", "")
                                rsu_port = endpoint.get("port", 0)
                                topics_list = rsu_topic.get("topics", [])
                                
                                print(f"\n  RSU: {rsu_ip}:{rsu_port}")
                                for topic in topics_list:
                                    topic_name = topic.get("name", "")
                                    is_selected = topic.get("selected", False)
                                    status_icon = "✓" if is_selected else "✗"
                                    print(f"    {status_icon} {topic_name}")
                            
                            print("\n✓ Topics successfully updated")
                        else:
                            print("\n⚠ No RSU topics in response")
                    else:
                        # Legacy format with status/message
                        status = response_json.get("status", "unknown")
                        message = response_json.get("message", "")
                        
                        print(f"Status: {status}")
                        if message:
                            print(f"Message: {message}")
                        
                        if status == "success" or status == "ok":
                            print("\n✓ Topics successfully selected for RSU")
                        else:
                            print("\n✗ Topic selection may have failed")
                
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
            print("  - RSU endpoint is not recognized")
            print("  - NATS server is not properly configured")
            return None
            
    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()
        return None
        
    finally:
        # Close connection
        if nc.is_connected:
            await nc.close()
            print("Disconnected from NATS server")


def main():
    parser = argparse.ArgumentParser(
        description='Send NATS request to select topics for an RSU',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Select topics for RSU at 127.0.0.1 (port is optional)
  python3 send_nats_request_select_topics.py --rsu-ip 127.0.0.1 \\
      --topics J2735_BSM_MessageReceiver
  
  # Select multiple topics (with optional port)
  python3 send_nats_request_select_topics.py --rsu-ip 127.0.0.1 --rsu-port 1610 \\
      --topics J2735_BSM_MessageReceiver J2735_MAP_MessageReceiver J2735_SPAT_MessageReceiver
  
  # Connect to remote NATS server with custom unit ID
  python3 send_nats_request_select_topics.py --nats-url nats://192.168.1.100:4222 \\
      --unit-id MyUnit --rsu-ip 10.0.0.1 \\
      --topics J2735_BSM_MessageReceiver
        """
    )
    
    parser.add_argument(
        '--nats-url',
        default='nats://localhost:4222',
        help='NATS server URL (default: nats://localhost:4222)'
    )
    
    parser.add_argument(
        '--unit-id',
        default='Unit001',
        help='Unit ID (default: Unit001)'
    )
    
    parser.add_argument(
        '--rsu-ip',
        default='127.0.0.1',
        help='RSU IP address (default: 127.0.0.1). This is the key used for topic selection.'
    )
    
    parser.add_argument(
        '--rsu-port',
        type=int,
        default=1610,
        help='RSU port number (optional, default: 1610). Note: System only uses RSU IP for matching.'
    )
    
    parser.add_argument(
        '--topics',
        nargs='+',
        default=['J2735_BSM_MessageReceiver'],
        help='List of topic names to select (space-separated, default: J2735_BSM_MessageReceiver)'
    )
    
    parser.add_argument(
        '--timeout',
        type=float,
        default=5.0,
        help='Request timeout in seconds (default: 5.0)'
    )
    
    args = parser.parse_args()
    
    print("=" * 60)
    print("NATS Request: Select RSU Topics")
    print("=" * 60)
    print()
    
    # Run async request
    response = asyncio.run(send_select_topics_request(
        args.nats_url, 
        args.unit_id, 
        args.rsu_ip,
        args.rsu_port,
        args.topics,
        args.timeout
    ))
    
    if response:
        return 0
    else:
        return 1


if __name__ == '__main__':
    sys.exit(main())
