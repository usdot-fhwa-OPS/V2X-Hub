# queue length estimation
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import math
import trajectory_construction_v1
import shockwave_estimation_v1

# parameters
current_time = 880
signal_input_address = r'C:\Users\ghiasia\Documents\Projects\18 - 327 OpsIV V2X Hub\Metrics\Code\Inputs for queue length estimation\Signal.csv'
traj_input_address = r'C:\Users\ghiasia\Documents\Projects\18 - 327 OpsIV V2X Hub\Metrics\Code\Inputs for queue length estimation\Traj.csv'
link_num = 1
lane_num = 1
signal_loc = 1151
default_shock1_sp = -4 # ft/s, this depends on traffic demand
default_shock2_sp = -35 # ft/s
stop_sp_range = 3.3 # ft/s

traj = trajectory_construction_v1.trajectory(traj_input_address)

class Signal:
    time = []
    status = []
    
def read_SPaT(signal_input_address,link_num,lane_num,current_time):
    with open(signal_input_address) as csv_file:
        csv_reader = pd.read_csv(csv_file)
        signal_all = csv_reader.values
        signal_status = Signal()
        for row in range(len(signal_all)):
            if signal_all[row][1]==link_num and signal_all[row][2]==lane_num:
                if (signal_all[row][0]>current_time):
                    break
                signal_status.time.append(signal_all[row][0])
                signal_status.status.append(signal_all[row][3])
        current_signal = signal_status.status[-1]
    return signal_status,current_signal    
                
def find_last_green(signal_status):
    i = 1
    for t in reversed(signal_status.time):
        if signal_status.status[-i] == 'GREEN':
            last_green_time = t
            break
        i += 1
        
    return last_green_time

def add_to_plot(a1,w1,a2,w2,stop_stat,signal_loc,Q,last_point_time,last_point_loc):
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

        
signal_status,current_signal = read_SPaT(signal_input_address,link_num,lane_num,current_time)
last_green_time = find_last_green(signal_status)
# plot_traj(traj,last_green_time,current_time)
a1,w1,a2,w2,stop_stat = shockwave_estimation_v1.shockwave(traj,last_green_time,current_time,current_signal,default_shock1_sp,default_shock2_sp,stop_sp_range)
last_point_time = max(stop_stat.time)
last_point_index = np.argmax(stop_stat.time)
last_point_loc = stop_stat.loc[last_point_index]
# correct R
R = 10
Qend_bar = math.inf
if current_signal == 'GREEN':
    Qend_bar = (-a1*w2+w1*w2*R+a2*w1)/(w1-w2)
Qend = min(Qend_bar,last_point_loc+w1*(current_time-last_point_time))
Q = signal_loc - Qend

add_to_plot(a1,w1,a2,w2,stop_stat,signal_loc,Q,last_point_time,last_point_loc)
print(Qend)
print(Q)

plt.show()
