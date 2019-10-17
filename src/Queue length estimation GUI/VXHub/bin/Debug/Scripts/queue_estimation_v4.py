# queue length estimation
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import math 
import sys
import csv
import trajectory_construction_v1
import shockwave_estimation_v1

# parameters
cinput_folder = sys.argv[1]
coutput_folder = sys.argv[2]
specific_time = sys.argv[3]
current_time = float(sys.argv[4])
time_interval = float(sys.argv[5])
default_shock1_sp = float(sys.argv[6])
default_shock2_sp = float(sys.argv[7])
stop_sp_range = float(sys.argv[8])

# print(specific_time)
# print(time_interval)

# temp to debug
# cinput_folder = r"C:\Users\ghiasia\Documents\Projects\18 - 327 OpsIV V2X Hub\Metrics\Code\Inputs for queue length estimation"
# coutput_folder = r"C:\Users\ghiasia\Documents\Projects\18 - 327 OpsIV V2X Hub\Metrics\Code\Outputs of queue length estimation"
# specific_time = False
# time_interval = 50
# current_time = 880
# default_shock1_sp = -4
# default_shock2_sp = -35
# stop_sp_range = 3.2

class Signal:
    time = []
    status = []
class Queue_all:
    link = []
    lane = []
    time = []
    queue = []
    
def read_SPaT(signal_input_address,link_num,lane_num,current_time,specific_time):
    with open(signal_input_address) as csv_file:
        csv_reader = pd.read_csv(csv_file)
        signal_all = csv_reader.values
        signal_status = Signal()
        for row in range(len(signal_all)):
            if signal_all[row][1]==link_num and signal_all[row][2]==lane_num:
                if (signal_all[row][0]>current_time) and specific_time==True:
                    break
                signal_status.time.append(signal_all[row][0])
                signal_status.status.append(signal_all[row][3])
        last_time = signal_all[-1][0]
    return signal_status,last_time

def read_Signalheads(sgheads_input_address,link_num,lane_num):
    with open(sgheads_input_address) as csv_file:
        csv_reader = pd.read_csv(csv_file)
        signal_heads = csv_reader.values
        for row in range(len(signal_heads)):
            if int(signal_heads[row][0])==link_num and int(signal_heads[row][1])==lane_num:
                signal_head_loc = signal_heads[row][2]
                break
    return signal_head_loc    

# All times: change this function              
def find_last_signal(signal_status):
    i = 1
    last_red_time = math.inf
    last_green_time = math.inf
    R = math.inf
    for t in reversed(signal_status.time):
        if i>len(signal_status.status):
            break
        if current_signal=='GREEN':
            if signal_status.status[-i] == 'RED':
                last_red_time = t
                break
            i += 1
        else:
            if signal_status.status[-i] == 'GREEN':
                last_green_time = t
                break
            i += 1
    # if signal is green, we find the last red interval (R)
    # time[-i] is already at the last_red_time
    if current_signal=='GREEN':
        while i<=len(signal_status.status):
            if signal_status.status[-i] == 'AMBER':
                R = last_red_time-signal_status.time[-i]
                break
            i += 1
        last_green_time = last_red_time-R
    return last_green_time,last_red_time,R

def add_to_plot(a1,w1,a2,w2,stop_stat,signal_loc,Q,last_point_time,last_point_loc):
    # All times: we don't need this function
    ts = np.linspace(last_green_time,current_time,num=10)
    ys = a1+w1*np.array(ts)
    plt.plot(ts,ys,linestyle='--',ms=0.75,color='0.75')
    ts = np.linspace(last_point_time,current_time,num=10)
    ys = last_point_loc+w1*np.array(ts-last_point_time)       
    plt.plot(ts,ys,linestyle='--',ms=1,color='r')
    plt.scatter(stop_stat.time,stop_stat.loc, marker='*',c='r')
    plt.arrow(current_time,signal_loc,0,-Q)
    Q_round = round(Q,0)
    plt.text(current_time-5,signal_loc-Q/2,str(Q_round)+' ft',fontsize=14)     
    plt.xlim(last_green_time,current_time+0.1)
    plt.ylim(0,signal_loc)
    plt.ioff()
    
    plt.show(block = False)

##### MAIN
signal_input_address = cinput_folder +'\Signal.csv'
sgheads_input_address = cinput_folder +'\signal_heads.csv'
# All times: change the file name, then each time interval should be reported in different sheets
if specific_time==True:
    output_address = coutput_folder +'\queue' + str(round(current_time)).zfill(3) + '.csv'
else:
    output_address = coutput_folder +'\queue' + '.csv'

with open(sgheads_input_address) as csv_file:
    csv_reader = pd.read_csv(csv_file)
    signal_heads = csv_reader.values
    num_traj = signal_heads.shape[0]

Q_all = Queue_all()
for traj_i in range(num_traj):
    traj_input_address = cinput_folder + '\Traj' + str(traj_i+1).zfill(2) + '.csv'
    traj = trajectory_construction_v1.trajectory(traj_input_address)
    link_num = traj.link
    lane_num = traj.lane
    signal_loc = read_Signalheads(sgheads_input_address,link_num,lane_num)
    signal_status,last_time = read_SPaT(signal_input_address,link_num,lane_num,current_time,specific_time)
    if specific_time==True:
        
        current_signal = signal_status.status[-1]
        last_green_time,last_red_time,R = find_last_signal(signal_status)
        a1,w1,a2,w2,stop_stat,move_stat = shockwave_estimation_v1.shockwave(traj,last_green_time,current_time,current_signal,default_shock1_sp,default_shock2_sp,stop_sp_range)
        if len(stop_stat.time)==0:
            last_point_time = last_green_time
            last_point_loc = signal_loc
        else:
            last_point_time = max(stop_stat.time)
            last_point_index = np.argmax(stop_stat.time)
            last_point_loc = stop_stat.loc[last_point_index]
        Qend_bar = math.inf
        if current_signal == 'GREEN':
            Qend_bar = (-a1*w2+w1*w2*R+a2*w1)/(w1-w2)
        Qend = min(Qend_bar,last_point_loc+w1*(current_time-last_point_time))
        Q = round(signal_loc - Qend)
        Q_all.link.append(link_num)
        Q_all.lane.append(lane_num)
        Q_all.time.append(current_time)
        Q_all.queue.append(Q)
    else:
        t0 = time_interval*math.ceil(signal_status.time[0]/time_interval)
        tend = time_interval*math.floor(last_time/time_interval)
        print(signal_status.time[-1])
        num_times = (tend-t0)/time_interval+1
        times_all = np.linspace(t0,tend,num_times)
        signal_partial = Signal()
        for i in range(len(times_all)):
            current_time = times_all[i]
            index = signal_status.time.index(current_time)
            current_signal = signal_status.status[index]
            signal_partial.status = signal_status.status[0:index]
            signal_partial.time = signal_status.time[0:index+1]
            last_green_time,last_red_time,R = find_last_signal(signal_partial)
            a1,w1,a2,w2,stop_stat,move_stat = shockwave_estimation_v1.shockwave(traj,last_green_time,current_time,current_signal,default_shock1_sp,default_shock2_sp,stop_sp_range)    
            if len(stop_stat.time)==0:
                last_point_time = last_green_time
                last_point_loc = signal_loc
            else:
                last_point_time = max(stop_stat.time)
                last_point_index = np.argmax(stop_stat.time)
                last_point_loc = stop_stat.loc[last_point_index]
            Qend_bar = math.inf
            if current_signal == 'GREEN':
                Qend_bar = (-a1*w2+w1*w2*R+a2*w1)/(w1-w2)
            Qend = min(Qend_bar,last_point_loc+w1*(current_time-last_point_time))
            Q = round(signal_loc - Qend)
            Q_all.link.append(link_num)
            Q_all.lane.append(lane_num)
            Q_all.time.append(current_time)
            Q_all.queue.append(Q)
    # add_to_plot(a1,w1,a2,w2,stop_stat,signal_loc,Q,last_point_time,last_point_loc)
    # print(Qend)
    # print(Q)

    # plt.show()
    
# print(Q_all.time)
with open(output_address, "w", newline='') as fp:
    writer = csv.writer(fp)
    writer.writerow(['link','lane','time','queue (ft)'])
    for row in range(len(Q_all.queue)-1):
        writer.writerow([str(Q_all.link[row]),str(Q_all.lane[row]),str(Q_all.time[row]),str(Q_all.queue[row])])
