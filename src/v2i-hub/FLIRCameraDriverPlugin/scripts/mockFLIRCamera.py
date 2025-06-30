import asyncio
import websockets
import json
import argparse
from dataclasses import dataclass, field
from datetime import datetime
import pytz
import random
timezone = pytz.timezone("America/New_York")


@dataclass
class FLIRDetection:
    """Class used to store data for each FLIR Detection
    """
    x: float
    y: float
    angle: float
    className: str
    speed: float
    iD: int
    latitude: float
    longitude: float

    def to_json(self):
        """Convert the FLIRDetection object to a JSON serializable dictionary."""
        return {
            "x": self.x,
            "y": self.y,
            "angle": self.angle,
            "class": self.className,
            "speed": self.speed,
            "iD": self.iD,
            "latitude": self.latitude,
            "longitude": self.longitude
        }

@dataclass
class FLIRPedestrianPresenceTracking:
    """Class used to store data for FLIR Pedestrian Presence Tracking
        Sample Pededertian Presence Tracking Data looks like: 
        message = {
            "dataNumber": "199262",
            "messageType": "Data",
            "time": "2025-05-20T13:47:35.092-04:00",
            "track": [
                {
                "angle": "255.00000000",
                "class": "Pedestrian",
                "iD": "2910604",
                "latitude": "38.95504354",
                "longitude": "-77.14934177",
                "speed": "1.12053359",
                "x": "-2.66856720",
                "y": "19.92332193"
                },
                {
                "angle": "246.00000000",
                "class": "Pedestrian",
                "iD": "2910927",
                "latitude": "38.95503376",
                "longitude": "-77.14937300",
                "speed": "4.96141529",
                "x": "-3.02072971",
                "y": "22.81371546"
                }
            ],
            "type": "PedestrianPresenceTracking"
                    
        }
    """
   
    track: list[FLIRDetection] = field(default_factory=list)
    dataNumber: int = 0
    time: datetime = datetime.now(timezone)

    def to_json(self):
        """Convert the FLIRPedestrianPresenceTracking object to a JSON serializable dictionary."""
        return {
            "dataNumber": self.dataNumber,
            "messageType": "Data",
            "time":self.format_time(),
            "track": [detection.to_json() for detection in self.track],
            "type": "PedestrianPresenceTracking"
        }
    def format_time(self):
        # Format the time with milliseconds and offset
        formatted_time = self.time.strftime('%Y-%m-%dT%H:%M:%S.%f')[:-3] + self.time.strftime('%z')
        # Insert colon in timezone offset
        formatted_time = formatted_time[:-5] + formatted_time[-5:-2] + ':' + formatted_time[-2:]
        return formatted_time
    
    def random_update(self):
        """Randomly update the FLIRPedestrianPresenceTracking object with new data."""
        # Update time 
        self.time = datetime.now(timezone)
        # Update dataNumber
        self.dataNumber += 1
        for detection in self.track:
            detection.x += random.uniform(-0.1, 0.1)
            detection.y += random.uniform(-0.1, 0.1)
            detection.angle += random.uniform(-5, 5)
            detection.speed = max(0, detection.speed + random.uniform(-0.5, 0.5))
            detection.latitude += random.uniform(-0.00001, 0.00001)
            detection.longitude += random.uniform(-0.00001, 0.00001)



async def sendMessage(websocket):
    """Send the JSON message to the client periodically."""
    while True:
        tracks= FLIRPedestrianPresenceTracking()
        # Ramdom chance to add a new track
        if len(tracks.track) < 5 and random.random() > 0.2:
            new_detection = FLIRDetection(
                x=random.uniform(-5, 5),
                y=random.uniform(-5, 5),
                angle=random.uniform(0, 360),
                className="Pedestrian",
                speed=random.uniform(0, 5),
                iD=random.randint(1000000, 9999999),
                latitude=random.uniform(38.95, 38.96),
                longitude=random.uniform(-77.15, -77.14)
            )
            tracks.track.append(new_detection)
        # Randomly remove a track
        if len(tracks.track) > 0 and random.random() < 0.1:
            if len(tracks.track) > 1:
                tracks.track.pop(random.randint(0, len(tracks.track) - 1))
            else:
                tracks.track.clear()
        # Update tracks
        tracks.random_update()
        if len(tracks.track) == 0:
            print("No tracks to send, skipping message")
        else:
            await websocket.send(json.dumps(tracks.to_json()))
            print(f"Sending: {tracks.to_json()}")
            await asyncio.sleep(0.1)  # interval to send the message

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
    parser = argparse.ArgumentParser(description='Script to mock detection data coming from FLIR Sensor. It will create and update a list' \
    'of randomly generated FLIRDetection objects (Up to 5 concurrent detections) and send them to the client periodically.')
    parser.add_argument('--ip', help='IP address to send detection data to.', type=str, default="127.0.0.1") 
    parser.add_argument('--port', help='Port to send detection data to.', type=int, default=9000)
    args = parser.parse_args()

    server = await websockets.serve(handleClient, args.ip, args.port)
    printStr = "WebSocket server started on ws://" + args.ip + ":" + str(args.port) + "/api/subscriptions"
    print(printStr)
    await server.wait_closed()

if __name__ == "__main__":
    asyncio.run(main())
