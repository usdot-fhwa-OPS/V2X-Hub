from binascii import hexlify, unhexlify
import J2735
import math
import json
import sys
import os
import pandas as pd
import numpy as np
import csv

class Signal:
    time = []
    status = []

def main_func(input_address): 
    signal = Signal()
    signal.time = []
    signal.status = []
    msg = J2735.DSRC.MessageFrame
    prev_index = 0
    for file_name in os.listdir(input_address):
        if os.path.isfile(os.path.join(input_address,file_name)) and 'SPaT' in file_name:
            file_address = input_address + '\\' + file_name
            with open(file_address) as csv_file:
                csv_reader = pd.read_csv(csv_file, skiprows=1)
                values_all = csv_reader.values
                num_points = len(values_all)
                time = values_all[:,0] # int
                status_raw = values_all[:,1] # str
                msg.from_uper(unhexlify(status_raw[0]))
                msg()
                num_sg = len(msg._val['value'][1]['intersections'][0]['states'])
                status = np.full([len(values_all)-1,num_sg],'-',dtype=str)
                header_txt = ['time']
                for i in range(len(values_all)-1):
                    msg.from_uper(unhexlify(status_raw[i]))
                    msg()
                    for j in range(num_sg):
                        if i==0 and prev_index==0:
                            header_txt.append(str(msg._val['value'][1]['intersections'][0]['states'][j]['signalGroup']))
                        value = msg._val['value'][1]['intersections'][0]['states'][j]['state-time-speed'][0]['eventState']
                        if value == 'permissive-Movement-Allowed' or value == 'protected-Movement-Allowed':
                            status[i,j] = 'GREEN'
                        elif value == 'permissive-clearance' or value == 'protected-clearance':
                            status[i,j] = 'YELLOW'
                        elif value == 'stop-And-Remain':
                            status[i,j] = 'RED'
                        else:
                            status[i,j] = '-'

                signal.time[prev_index+1:prev_index+num_points] = time
                signal.status[prev_index+1:prev_index+num_points] = status
                prev_index = prev_index+num_points
    signal = list(zip(signal.time,signal.status))
    signal.sort(key = lambda signal_new: signal_new[0])
    return signal