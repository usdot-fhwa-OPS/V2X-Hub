# queue length and average delay estimation
import pandas as pd
import numpy as np
import math 
import sys
import os
import json
import geopy.distance
import csv
import SPaT_convertor_v3
import BSM_convertor
import wgs84_to_utm
import longitudinal_traj
import shockwave_estimation_v2

# INPUTS:
# 1 - BSM file (in UPER Hex format; ONE file only)
# 2 - SPaT files (in UPER Hex format)
# Note that jumps in the signal timestamps may cause unrealistic queue length and average delay estimations
# 3 - MAP file (in geojson format)

class GPS_MAP():
    lat = []
    lon = []
    lane = []
class Signal_Heads():
    lat = []
    lon = []
    lane = []
class Trajectory():
    lat = []
    lon = []
    id = []
    ts = []
    vs = []
    x = []
    y = []
class Points():
    x = []
    y = []
class Signal():
    time = []
    status = []
class SG_Lane():
    SG = []
    lane = []
class Queue_all():
    lane = []
    time = []
    queue = []
class Delay():
    lane = []
    delay = []
def get_signal_status(signal,SG_lane,queue_time,delay_to,check_queue,check_delay,specific_time):
    signal_status = Signal()
    signal_status.time = []
    signal_status.status = []
    if check_queue == 'False':
        queue_time = 0
    if check_delay == 'False':
        delay_to = 0
    if specific_time == 'True':
        max_time = max(queue_time,delay_to)
    else:
        max_time = math.inf
    for row in range(len(signal)):
        if (signal[row][0]>max_time):
            break
        signal_status.time.append(signal[row][0])
        signal_status.status.append(signal[row][1])
    return signal_status
def find_last_signal(signal,SG,index_time,current_signal):
    i = 1
    last_red_time = math.inf
    last_green_time = math.inf
    R = math.inf
    time = signal['time'][0:index_time]
    for t in reversed(time):
        if current_signal=='G':
            if signal[str(SG)][index_time-i] == 'R':
                last_red_time = t
                break
            i += 1
        else:
            if signal[str(SG)][index_time-i] == 'G':
                last_green_time = t
                break
            i += 1
    # if signal is green, we find the last red interval (R)
    if current_signal=='G':
        while i<=index_time+1:
            if signal[str(SG)][index_time-i] == 'R':
                R = last_red_time-time[index_time-i]
                break
            i += 1
        last_green_time = last_red_time-R
    return last_green_time,last_red_time,R

if __name__ == '__main__':
    # parameters
    cinput_folder = sys.argv[1]
    coutput_folder = sys.argv[2]
    use_saved_data = sys.argv[3]
    check_queue = sys.argv[4]
    check_delay = sys.argv[5]
    specific_time = sys.argv[6]
    queue_time = float(sys.argv[7])
    queue_interval = float(sys.argv[8])
    delay_from = float(sys.argv[9])
    delay_to = float(sys.argv[10])
    default_shock1_sp = float(sys.argv[11])
    default_shock2_sp = float(sys.argv[12])
    stop_sp_range = float(sys.argv[13])
    jam_spacing = float(sys.argv[14])
    # if the minimum distance between each BSM and MAP GPS coordinates is greater than dist_threshold, the BSM coordinates are considered out of boundary
    dist_threshold = 3 # meters
    
    signal_address = cinput_folder + '\\signal_df.csv'
    lanes_address = cinput_folder + '\\lanes_df.csv'
    traj_address = cinput_folder + '\\traj_df.csv'
    SG_lane_address = cinput_folder + '\\SG_Lane.csv'
        
    if use_saved_data == 'False':
        # receive and reformat SPaT
        print('Reading signal data ...')
        signal = SPaT_convertor_v3.main_func(cinput_folder)
        print('Reading signal data completed!')
            
        # convert bsm gps points to longitudinal coordinate along the road
        traj = Trajectory()
        map_gps = GPS_MAP()
        signal_heads = Signal_Heads()
        SG_lane = SG_Lane()
        bsm_num = 0
        for file_name in os.listdir(cinput_folder):
            # MAP
            if file_name.endswith('.geojson'):
                print('Reading MAP data ...')
                MAP_address = cinput_folder + '\\' + file_name
                with open(MAP_address) as f:
                    map_data = json.load(f)
                num_lanes = len(map_data['lanes']['features'])
                for l in range(num_lanes):
                    # skip Engress lanes
                    if map_data['lanes']['features'][l]['properties']['connections'][0]['signal_id']=='':
                        continue
                    lane_num = int(map_data['lanes']['features'][l]['properties']['laneNumber'])
                    signal_heads.lon.append(map_data['lanes']['features'][l]['properties']['elevation'][0]['latlon']['lon'])
                    signal_heads.lat.append(map_data['lanes']['features'][l]['properties']['elevation'][0]['latlon']['lat'])
                    signal_heads.lane.append(lane_num)
                    # maping the signal groups to lanes
                    SG_lane.lane.append(lane_num)
                    SG_lane.SG.append(int(map_data['lanes']['features'][l]['properties']['connections'][0]['signal_id']))
                    num_points = len(map_data['lanes']['features'][l]['properties']['elevation'])
                    
                    for i in range(num_points):
                        # signal head location is assumed to be equal to the first point of each lane    
                        map_gps.lon.append(map_data['lanes']['features'][l]['properties']['elevation'][i]['latlon']['lon'])
                        map_gps.lat.append(map_data['lanes']['features'][l]['properties']['elevation'][i]['latlon']['lat'])
                        map_gps.lane.append(lane_num)
                print('Reading MAP data completed!')
            # BSM (For this code, BSM records should be placed into one file, but this can be easily fixed if needed)
            if 'BSM' in file_name:
                print('Reading BSM data ...')
                file_address = cinput_folder + '\\' + file_name
                traj = BSM_convertor.main_func(file_address)
                print('Reading BSM data completed!')
        
        map_points = wgs84_to_utm.main(map_gps.lat,map_gps.lon)
        traj_temp = wgs84_to_utm.main(traj.lat,traj.lon)
        traj.x = traj_temp.x
        traj.y = traj_temp.y        

        # get all signals up to max_time (max_time is determined according to the user-selected settings)
        print('Converting signal status data ...')
        signal_status = get_signal_status(signal,SG_lane,queue_time,delay_to,check_queue,check_delay,specific_time)
        print('Converting signal status data completed!')

        # the object arrays are converted to dataframes and then saved as csv files to be used later in the "Use post-processed data" option
        print('Saving signal data into csv files ...')
        col_names = ['time']
        signal_df = pd.DataFrame(columns=col_names)
        signal_df['time'] = signal_status.time
        for i in range(len(SG_lane.lane)):
            col_names.append(str(SG_lane.SG[i]))
            temp_data = []
            for j in range(len(signal_status.status)):
                temp_data.append(signal_status.status[j][i]) 
            signal_df[col_names[-1]] = temp_data
        signal_df.to_csv(signal_address,index = None, header=True)
        lanes_all = np.unique(signal_heads.lane)
        lanes_df = pd.DataFrame(lanes_all)
        lanes_df.to_csv(lanes_address,index = None, header=False)
        col_names = ['Signal group','Lane number']
        SG_lane_df = pd.DataFrame(columns=col_names)    
        SG_lane_df['Signal group'] = SG_lane.SG
        SG_lane_df['Lane number'] = SG_lane.lane
        SG_lane_df.to_csv(SG_lane_address,index = None, header=True)
        print('Saving signal data into csv files completed!')
    else:
        print('Reading signal data from post-processed csv files ...')
        signal_df = pd.read_csv(signal_address)
        with open(lanes_address) as f:
            reader = csv.reader(f)
            lanes_all = list(reader)
        for i in range(len(lanes_all)):
            lanes_all[i] = int(lanes_all[i][0])
        SG_lane_df = pd.read_csv(SG_lane_address)
        print('Reading signal data from post-processed csv files completed!')
                
    col_names = ['id','ts','vs','ps']
    q_all = Queue_all()
    d_all = Delay()
    
    for direction in range(len(lanes_all)):
        lane_num = lanes_all[direction]
        file_name = 'traj_df_' + str(lane_num).zfill(2) + '.csv'
        traj_address = cinput_folder + '\\' + file_name

        if use_saved_data == 'False':
            print('Creating trajectories and saving them into a csv file for lane ' + str(lane_num) + ' ...')
            traj_cum_df = pd.DataFrame(columns=col_names)
            index_lane_signal = np.where(signal_heads.lane==lane_num)[0][0]
            index_lane = np.where(map_gps.lane==lane_num)[0]
            direction_points = Points()
            direction_points.x = map_points.x[index_lane]
            direction_points.y = map_points.y[index_lane]
            signal_loc = wgs84_to_utm.main([signal_heads.lat[index_lane_signal]],[signal_heads.lon[index_lane_signal]])
            col_name = str(lane_num)
            traj_cum = longitudinal_traj.main_func(direction_points,traj,signal_loc,dist_threshold)
            traj_cum_df['id'] = traj_cum.id
            traj_cum_df['ts'] = traj_cum.ts
            traj_cum_df['vs'] = traj_cum.vs
            traj_cum_df['ps'] = traj_cum.ps
            traj_cum_df.to_csv(traj_address,index = None, header=True)
            print('Creating trajectories and saving them into a csv file for lane ' + str(lane_num) + ' completed!')
        else:
            traj_cum_df = pd.read_csv(traj_address)
            print('Reading trajectory data from the post-processed csv file for lane ' + str(lane_num) + ' completed!')
        
        index_lane = np.where(SG_lane_df['Lane number']==lane_num)[0][0]      
        
        if check_queue == 'True':
            print('Calculating queue length for lane ' + str(lane_num) + ' ...')
            if specific_time == 'True':
                index_time = next((time for time, val in enumerate(signal_df['time']) if val>=queue_time),-1)
                current_signal = signal_df[str(SG_lane_df['Signal group'][index_lane])][index_time]
                print(file_name)
                last_green_time,last_red_time,R = find_last_signal(signal_df,SG_lane_df['Signal group'][index_lane],index_time,current_signal)
                a1,w1,a2,w2,stop_stat,move_stat = shockwave_estimation_v2.shockwave(traj_cum_df,last_green_time,queue_time,current_signal,default_shock1_sp,default_shock2_sp,stop_sp_range)
                if w2-w1 == 0:
                    Q = 0
                else:
                    if len(stop_stat.time)==0:
                        last_point_time = last_green_time
                        last_point_loc = 0
                    else:
                        last_point_time = max(stop_stat.time)
                        last_point_index = np.argmax(stop_stat.time)
                        last_point_loc = stop_stat.loc[last_point_index]
                    Qend_bar = math.inf
                    if current_signal == 'G':
                        Qend_bar = (-a1*w2+w1*w2*R+a2*w1)/(w1-w2)
                    Qend = min(Qend_bar,last_point_loc+w1*(queue_time-last_point_time))
                    if Qend<math.inf:
                        Q = round(Qend)
                    else:
                        Q = 'NaN'
                q_all.lane.append(lane_num)
                q_all.time.append(queue_time)
                q_all.queue.append(Q)
            else:
                t0 = queue_interval*math.ceil(min(signal_df['time'])/queue_interval)
                tend = queue_interval*math.floor(max(signal_df['time'])/queue_interval)
                num_times = (tend-t0)/queue_interval+1
                times_all = np.linspace(t0,tend,num_times)
                for i in range(len(times_all)):
                    current_time = times_all[i]                    
                    index_time = next((time for time, val in enumerate(signal_df['time']) if val>=current_time),-1)
                    current_signal = signal_df[str(SG_lane_df['Signal group'][index_lane])][index_time]
                    last_green_time,last_red_time,R = find_last_signal(signal_df,SG_lane_df['Signal group'][index_lane],index_time,current_signal)
                    a1,w1,a2,w2,stop_stat,move_stat = shockwave_estimation_v2.shockwave(traj_cum_df,last_green_time,current_time,current_signal,default_shock1_sp,default_shock2_sp,stop_sp_range)    
                    if w2-w1 == 0:
                        Q = 0
                    else:
                        if len(stop_stat.time)==0:
                            last_point_time = last_green_time
                            last_point_loc = 0
                        else:
                            last_point_time = max(stop_stat.time)
                            last_point_index = np.argmax(stop_stat.time)
                            last_point_loc = stop_stat.loc[last_point_index]
                        Qend_bar = math.inf
                        if current_signal == 'G':
                            Qend_bar = (-a1*w2+w1*w2*R+a2*w1)/(w1-w2)
                        Qend = min(Qend_bar,last_point_loc+w1*(current_time-last_point_time))
                        if Qend<math.inf:
                            Q = round(Qend)
                        else:
                            Q = 'NaN'
                    q_all.lane.append(lane_num)
                    q_all.time.append(current_time)
                    q_all.queue.append(Q)
            print('Calculating queue length for lane ' + str(lane_num) + ' completed!')
            
        if check_delay == 'True':
            print('Calculating average delay for lane ' + str(lane_num) + ' ...')
            delay_total = 0
            # calculate delay during each signal cycle between times delay_from and delay_to (assumes cycle starts with red interval)
            previous_signal = '-'
            time_from = int(max(min(signal_df['time']),delay_from))
            time_to = int(min(max(signal_df['time']),delay_to))
            last_green_time = time_from
            # check at every 1 second
            for time in range(time_from,time_to):
                index_time = next(i for i, val in enumerate(signal_df['time']) if val>=time)
                current_signal = signal_df[str(SG_lane_df['Signal group'][index_lane])][index_time]    
                if (previous_signal=='G' or previous_signal=='Y') and current_signal=='R':
                    # shockwave
                    a1,w1,a2,w2,stop_stat,move_stat = shockwave_estimation_v2.shockwave(traj_cum_df,last_green_time,time-1,previous_signal,default_shock1_sp,default_shock2_sp,stop_sp_range)    
                    # delay area
                    if w2-w1 == 0:
                        delay = 0
                    else:
                        Q_bar = (a1*w2+w1*w2*R-a2*w1)/(w2-w1)
                        delay_area = R*Q_bar/2
                        delay = delay_area/jam_spacing
                    delay_total += delay
                    last_green_time = time
                if previous_signal=='R' and current_signal=='G':
                    R = time - last_green_time
                previous_signal = current_signal
                
            d_all.lane.append(lane_num)
            d_all.delay.append(delay_total)
            print('Calculating average delay for lane ' + str(lane_num) + ' completed!')
    
    # output
    print('Creating outputs ...')
    if check_queue == 'True':
        if specific_time == 'True':
            output_address = coutput_folder +'\queue' + str(round(queue_time)).zfill(3) + '.csv'
        else:
            output_address = coutput_folder +'\queue' + '.csv'                       
        with open(output_address, "w", newline='') as fp:
            writer = csv.writer(fp)
            writer.writerow(['lane','time','queue (ft)'])
            for row in range(len(q_all.queue)-1):
                writer.writerow([str(q_all.lane[row]),str(q_all.time[row]),str(q_all.queue[row])])
    if check_delay == 'True':
        output_address = coutput_folder +'\delay_' + str(round(delay_from)) + '_' + str(round(delay_to)) + '.csv'
        with open(output_address, "w", newline='') as fp:
            writer = csv.writer(fp)
            writer.writerow(['lane','total delay (s)'])
            for row in range(len(d_all.delay)-1):
                writer.writerow([str(d_all.lane[row]),str(d_all.delay[row])])
        