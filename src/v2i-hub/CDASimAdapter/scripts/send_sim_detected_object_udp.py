
import socket
import sys
import json
import time

# Script for integration testing CDASimAdapter Time Sync functionality.
# This python script sends periodic time sync messages to a configurable
# host and port. To Test the Time Sync functionality of the CDASimAdapter
# set port to the value of the TIME_SYNC_PORT environment variable.
#
# TODO Move this script into a more permanent location
count_num = 0

def generate_sim_external_object():
    jsonResult = {
        "metadata": {
            "type": "Application",
            "subtype": "SensorDetectedObject",
            "timestamp": 123,
            "isSimulated": True
        },
        "payload": {
            "sensor_id": "sensor1",
            "proj_string": "asdlasdkasd",
            "type": "Car",
            "confidence": "0.7",
            "objectId": "Object1",
            "position": {
                "x": 1.0,
                "y": 2.5,
                "z": 1.1
            },
            "positionCovariance" : [12,12,2, 34, 34, 55],
            "velocity": {
                "x": 1.0,
                "y": 2.5,
                "z": 1.1
            },
            "velocityCovariance" : ["a11", "a12", "a13", "a21", "a22", "a23", "a31", "a32", "a33"],
            "angularVelocity":{
                "x": 1.0,
                "y": 2.5,
                "z": 1.1
            },
            "angularVelocityCovariance" : ["a11", "a12", "a13", "a21", "a22", "a23", "a31", "a32", "a33"],
            "size": {
                "length": 0.1,
                "width": 0.4,
                "height": 1.5
            }     
        }
    }
    jsonResult = json.dumps(jsonResult)
    return jsonResult
port = 7576
address = "127.0.0.1"
host = (address, port)
try:
    sock = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
except socket.error as err:
    print('Socket error because of %s' %(err))


while True :
    try:
        msg = generate_sim_external_object()
        encoded_msg = str.encode(msg)
        count_num += 1
        sock.sendto(encoded_msg,host)
        print( encoded_msg.decode(encoding= 'UTF-8'), 'was sent to ', host)
        print(f'Message sent at ${time.time()}')
        time.sleep(5)
    except socket.gaierror:

        print ('There an error resolving the host')
        break
        
sock.close()


