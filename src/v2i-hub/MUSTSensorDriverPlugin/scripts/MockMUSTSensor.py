#!/usr/bin/python3

import socket
import time
import argparse
from dataclasses import dataclass
from enum import Enum
import random


class DetectionClassification(Enum):
    """Enumeration used for identifying type of detection
    """
    SEDAN='sedan'
    VAN='van'
    TRUCK='truck'
class DetectionSize(Enum):
    """Enumeration used for identifying the type of KafkaLogMessage
    """
    SMALL='small'
    MEDIUM='medium'
    LARGE='large'

@dataclass
class MUSTDetection:
    """Class used to store data for each Kafka Log Message
    """
    classification: DetectionClassification
    x: float
    y: float
    heading: float
    speed: float
    size: DetectionSize
    confidence: float
    track_id: int
    timestamp: int

    def to_csv(self):
        return f'{self.classification.value},{self.x},{self.y},{self.heading},{self.speed},{self.size.value},{self.confidence},{self.track_id},{self.timestamp}'

def update_detection(detection):
    """Function moves detection and heading by random increment and changes speed to a random value between 0 and 10.
    """
    detection.x = random.uniform(-1.0, 1.0) + detection.x
    detection.y = random.uniform(-1.0, 1.0) + detection.y
    detection.heading = random.uniform(-5.0, 5.0) + detection.heading
    detection.speed = random.uniform(0.0, 10)
    detection.timestamp = round(time.time())
     

def create_socket():       
    return socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    
def send_detection(sock, detection, host):
    try:
        msg = detection.to_csv()
        encoded_msg = str.encode(msg)
        sock.sendto(encoded_msg,host)
        print( encoded_msg.decode(encoding= 'UTF-8'), 'was sent to ', host)
    except socket.gaierror:
        print ('There an error resolving the host')

detections = []
detections.append(MUSTDetection(DetectionClassification.SEDAN, 0, 0, 330, 1, DetectionSize.MEDIUM, 95, 2,round(time.time())))

def main():
    parser = argparse.ArgumentParser(description='Script to mock detection data coming from MUST Sensor')
    parser.add_argument('--ip', help='IP address to send detection data to.', type=str, default="127.0.0.1") 
    parser.add_argument('--port', help='Port to send detection data to.', type=str, default=4545)  
    args = parser.parse_args()
    sock = create_socket()
    host = (args.ip, args.port)

    print("Mocking MUST Sensor detections ...")
    while True:
        for detection in detections:
            update_detection(detection)
            send_detection(sock,detection,host)
            time.sleep(1)

if __name__ == '__main__':
    main()