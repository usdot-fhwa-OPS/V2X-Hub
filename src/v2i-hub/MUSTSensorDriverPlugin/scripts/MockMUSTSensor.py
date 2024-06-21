#!/usr/bin/python3

import socket
import sys
import time
import argparse
from dataclasses import dataclass
from enum import Enum


class DetectionClassification(Enum):
    """Enumeration used for indentifying type of detection
    """
    SEDAN='sedan'
    VAN='van'
    TRUCK='truck'
class DetectionSize(Enum):
    """Enumeration used for indentifying the type of KafkaLogMessage
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

    def to_csv():
        return f'{self.classification},{self.x},{self.y},{self.heading},{self.speed},{self.size},{self.confidence},{self.track_id},{self.timstamp}'

def move_detection():
    return 

def create_socket():       
    try:
        return socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    except socket.error as err:
        print('Socket error because of %s' %(err))
def send_detection(sock, detection, host):
    try:
        msg = detection
        encoded_msg = str.encode(msg)
        sock.sendto(encoded_msg,host)
        print( encoded_msg.decode(encoding= 'UTF-8'), 'was sent to ', host)
    except socket.gaierror:
        print ('There an error resolving the host')

detections = []
detections.append(MUSTDetection(DetectionClassification.SEDAN, 0, 0, 330, 1, DetectionSize.MEDIUM, 95, 2,round(time.time() * 1000)))

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
            send_detection(sock,detection,host)

if __name__ == '__main__':
    main()