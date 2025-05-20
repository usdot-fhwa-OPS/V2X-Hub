import asyncio
import json
import websockets
import argparse
import csv


async def send_json_receive_json(uri, data):
    async with websockets.connect(uri) as websocket:
        # Send JSON data
        await websocket.send(json.dumps(data))
        print(f"Sent: {data}")

        # Continuously receive and decode JSON responses
        while True:
            try:
                response = await websocket.recv()
                decoded_response = json.loads(response)
                write_json_to_csv(response, decoded_response['track']['iD']+".csv")
            except websockets.exceptions.ConnectionClosed:
                print("Connection closed by server")
                break
            except json.JSONDecodeError:
                print(f"Received non-JSON message: {response}")
            except Exception as e:
                print(f"An error occurred: {e}")
                break

def write_json_to_csv(json_string, csv_filepath):
    """
    Writes JSON string data to a CSV file.

    Args:
        json_string (str): The JSON string data.
        csv_filepath (str): The path to the CSV file to write to.
    """
    try:
        data = json.loads(json_string)
    except json.JSONDecodeError as e:
         raise ValueError(f"Invalid JSON string: {e}")

    if not isinstance(data, list):
        data = [data]
    
    if data:
        header = data[0].keys()
        
        with open(csv_filepath, 'w', newline='') as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=header)
            writer.writeheader()
            for row in data:
                writer.writerow(row)
async def main():

    parser = argparse.ArgumentParser(description='Script to mock detection data coming from FLIR Sensor')
    parser.add_argument('--ip', nargs='+', help='IP address to send detection data to.', type=str, default="127.0.0.1") 
    parser.add_argument('--port',nargs='+', help='Port to send detection data to.', type=int, default=9000)
    args = parser.parse_args()
    tasks= []
    for ip, port in zip(args.ip, args.port):
        uri = "ws://" + args.ip + ":" + str(args.port) + "/api/subscriptions"  # Replace with your WebSocket server URI
        data_to_send = {"{\"messageType\":\"Subscription\", \"subscription\":{ \"type\":\"Data\", \"action\":\"Subscribe\", \"inclusions\":[{\"type\":\"PedestrianPresenceTracking\"}]}}"}
        asyncio.create_task(send_json_receive_json(uri, data_to_send))

    await asyncio.gather(*tasks)

    

if __name__ == "__main__":
    asyncio.run(main())