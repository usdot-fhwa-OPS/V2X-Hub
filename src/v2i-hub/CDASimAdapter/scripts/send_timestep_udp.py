
import socket
import sys
import json
import time

# Script for integration testing CDASimAdapter Time Sync functionality.
# This python script sends periodic time sync messages to a configurable
# host and port. To Test the Time Sync functionality of the CDASimAdapter
# set port to the value of the TIME_SYNC_PORT environment variable.
#
# count_name used to populate the seq field in the time sync message
count_num = 0
# sim_time (ms) used to populate the timestep field in the time sync message
sim_time =0
# sleep_dur (s) used to control the rate at which time sync messages are sent
sleep_dur = 0.1
def generate_time_sync():
    jsonResult = {
        "seq":count_num, 
        "timestep":sim_time
    }
    jsonResult = json.dumps(jsonResult)
    return jsonResult
port = 7575
address = "127.0.0.1"
host = (address, port)
try:
    sock = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
except socket.error as err:
    print('Socket error because of %s' %(err))


while True :
    try:
        msg = generate_time_sync()
        encoded_msg = str.encode(msg)
        count_num += 1
        sock.sendto(encoded_msg,host)
        print( encoded_msg.decode(encoding= 'UTF-8'), 'was sent to ', host)
        time.sleep(sleep_dur)
        sim_time+= 100
    except socket.gaierror:

        print ('There an error resolving the host')
        break
        
sock.close()


