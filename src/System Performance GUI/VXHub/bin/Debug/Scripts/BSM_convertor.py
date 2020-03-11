from binascii import hexlify, unhexlify
import J2735
import math
import os
import pandas as pd
import numpy as np
import csv

class Trajectory:
    id = []
    ts = []
    lon = []
    lat = []
    vs = []
    ac = []

def main_func(file_address):   
    traj = Trajectory()
    traj.id = []
    traj.ts = []
    traj.lon = []
    traj.lat = []
    traj.vs = []
    traj.ac = []
    msg = J2735.DSRC.MessageFrame
    prev_index = 0
    with open(file_address) as csv_file:
        csv_reader = pd.read_csv(csv_file, skiprows=1)
        bsm_data = csv_reader.values
        for i in range(len(bsm_data)-1):
            # first, check if bsm data record is correct (it sould start with 0014)
            # in some data records, some additional characters exist before '0014', which are being removed
            if bsm_data[i,1][0:4] != '0014':
                temp_str = bsm_data[i,1]
                index = temp_str.find('0014')
                if index == -1:
                    print('CORRUPT DATA RECORD')
                bsm_data[i,1] = temp_str[index:]
            msg.from_uper(unhexlify(bsm_data[i,1]))
            msg()
            coreData = msg._val['value'][1]['coreData']
            traj.id.append(int.from_bytes(coreData['id'], 'big'))
            traj.ts.append(bsm_data[i,0])
            num_len = int(math.log10(abs(coreData['long'])))+1
            traj.lon.append(coreData['long']/(10**(num_len-2)))
            num_len = int(math.log10(abs(coreData['lat'])))+1
            traj.lat.append(coreData['lat']/(10**(num_len-2)))
            if coreData['speed'] != 8191:
                traj.vs.append(coreData['speed']*0.02*3.28084)
            else:
                traj.vs.append('nan')
    return traj
                    