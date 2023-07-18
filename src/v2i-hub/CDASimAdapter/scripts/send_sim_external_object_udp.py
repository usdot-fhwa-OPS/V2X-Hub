
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
        "MetadataIsSimulation": False,
        "MetadataDatum": "",
        "MetadataProjString": "",
        "MetadataSensorX": "",
        "MetadataSensorY": "",
        "MetadataSensorZ": "",
        "MetadataInfrastructureId": "",
        "MetadataSensorId": "",
        "HeaderStampSecs": 1,
        "HeaderStampNsecs": 2,
        "Id": 10,
        "PosePosePositionX": 0.2,
        "PosePosePositionY": 0.3,
        "PosePosePositionZ": 0.4,
        "PosePoseOrientationX": 0.1,
        "PosePoseOrientationY": 0.2,
        "PosePoseOrientationZ": 0.3,
        "PosePoseOrientationW": 0.4,
        "BsmId": [12,12,22,11],
        "PoseCovariance": [
            1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0
        ],
        "VelocityTwistLinearX": 0.1,
        "VelocityTwistLinearY": 0.1,
        "VelocityTwistLinearZ": 0.1,
        "VelocityTwistAngularX": 0.1,
        "VelocityTwistAngularY": 0.1,
        "VelocityTwistAngularZ": 0.1,
        "VelocityCovariance": [
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0
        ],
        "SizeX": 0.3,
        "SizeY": 0.4,
        "SizeZ": 0.6,
        "Confidence": 0.0,
        "ObjectType": "",
        "DynamicObj": False
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


