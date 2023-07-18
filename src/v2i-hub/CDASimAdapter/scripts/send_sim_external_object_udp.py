
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
    jsonResult ={      
        "metadata":{
            "is_simulation": True,
            "datum": "WGS84",
            "proj_string": "epsg:3785",
            "sensor_x": 12.212,
            "sensor_y": 2.2121212,
            "sensor_z": 0.121212212,
            "infrastructure_id": "test",
            "sensor_id": "test_sensor"
        },
        "header": {
            "seq": 2,
            "stamp": {
            "secs": 3,
            "nsecs":4
            }
        },
        "id": 1212,
        "pose": {
            "pose": {
            "position": {
                "x": 12.0,
                "y": 23.0,
                "z": 56.121212
            },
            "orientation": {
                "x": 11.0,
                "y": 22.0,
                "z": 33.333333,
                "w": 44.4444444444
            }
            },
            "covariance": [
                12.12, 12.3233232, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0
            ]
        },
        "velocity": {
            "twist": {
            "linear": {
                "x": 12.1111111111111,
                "y": 2.11111112222,
                "z": 3.2222222222333
            },
            "angular": {
                "x": 2.212222222222,
                "y": 2.2333333333333,
                "z": 3.3333333333121
            }
            },
            "covariance": [
                12.2222222222, 123.33333333, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0,23.66666666666
            ]
        },
        "size": {
            "x": 12.0,
            "y": 23.0,
            "z": 23.0
        },
        "confidence": 12.0,
        "object_type": "2",
        "dynamic_obj": True
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


