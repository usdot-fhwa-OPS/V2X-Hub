import asyncio
import websockets
import json

# Sample Pededertian Presence Tracking Data
message = {
    "dataNumber": "473085",
    "messageType": "Data",
    "time": "2022-04-20T15:25:51.001-04:00",
    "track": [
        {
            "angle": "263.00000000",
            "class": "Pedestrian",
            "iD": "15968646",
            "latitude": "38.95499217",
            "longitude": "-77.14920953",
            "speed": "1.41873741",
            "x": "0.09458912",
            "y": "14.80903757",
        }
    ],
    "type": "PedestrianPresenceTracking",
}

async def sendMessage(websocket):
    """Send the JSON message to the client periodically."""
    while True:
        await websocket.send(json.dumps(message))
        print("Message sent to client")
        await asyncio.sleep(1)  # interval to send the message

async def handleClient(websocket, path):
    """Handle a new client connection."""
    if path == "/api/subscriptions":
        print(f"Client connected to path: {path}")
        try:
            await sendMessage(websocket)
        except websockets.exceptions.ConnectionClosed as e:
            print(f"Connection closed: {e}")
    else:
        print(f"Client attempted to connect to invalid path: {path}")
        await websocket.close(code=4000, reason="Invalid path")

async def main():
    # Start server at port 9000, as port 80 is used by V2X Hub
    server = await websockets.serve(handleClient, "127.0.0.1", 9000)
    print("WebSocket server started on ws://127.0.0.1:9000/api/subscriptions")
    await server.wait_closed()

if __name__ == "__main__":
    asyncio.run(main())
