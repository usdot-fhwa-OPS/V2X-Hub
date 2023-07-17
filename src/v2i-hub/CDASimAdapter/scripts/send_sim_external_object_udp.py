
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
        "metadata":{
            "is_simulation": False,
            "datum": "",
            "proj_string": "",
            "sensor_x": 0.0,
            "sensor_y": 0.0,
            "sensor_z": 0.0,
            "infrastructure_id": "",
            "sensor_id": ""
        },
        "header": {
            "seq": 0,
            "stamp": {
            "secs": 0,
            "nsecs": 0
            }
        },
        "id": 0,
        "pose": {
            "pose": {
            "position": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
            },
            "orientation": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0,
                "w": 0.0
            }
            },
            "covariance": [
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
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
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
            },
            "angular": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
            }
            },
            "covariance": [
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0
            ]
        },
        "size": {
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
        },
        "confidence": 0.0,
        "object_type": "",
        "dynamic_obj": False
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


