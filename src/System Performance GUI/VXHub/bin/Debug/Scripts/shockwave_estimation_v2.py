# Shockwave estimation module
import numpy as np
from sklearn.linear_model import LinearRegression

class Stopping_Stat:
    traj_id = []
    index = []
    time = []
    loc = []
class Moving_Stat:
    traj_id = []
    index = []
    time = []
    loc = []

def shockwave(traj,last_green_time,current_time,current_signal,default_shock1_sp,default_shock2_sp,stop_sp_range):   
    def focus_vehicles(traj,last_green_time,current_time):
        # sorting is not necessary as some vehicles may take over in the region of interest.
        focus_traj = []
        veh_all = np.unique(traj['id'])
        for n in veh_all:
            veh_index = np.where(traj['id']==n)[0]
            if traj['ts'][veh_index[0]]<current_time and traj['ts'][veh_index[-1]]>last_green_time:
                focus_traj.append(n)
        return focus_traj
    def update_veh_stat(traj,focus_traj,current_time,current_signal):
        stop_stat = Stopping_Stat()
        stop_stat.traj_id = []
        stop_stat.index = []
        stop_stat.time = []
        stop_stat.loc = []
        move_stat = Moving_Stat()
        move_stat.traj_id = []
        move_stat.index = []
        move_stat.time = []
        move_stat.loc = []  
        for n in focus_traj:
            veh_index = np.where(traj['id']==n)[0]
            for i in veh_index:
                if traj['ts'][i]>current_time:
                    break
                v = traj['vs'][i]
                if v<stop_sp_range:
                    stop_stat.traj_id.append(n)
                    stop_stat.index.append(i)
                    stop_stat.time.append(traj['ts'][i])
                    stop_stat.loc.append(traj['ps'][i])
                    break 
        if current_signal == 'G': 
            for n in focus_traj:
                veh_index = np.where(traj['id']==n)[0]
                for i in reversed(veh_index):
                    if traj['ts'][i]>current_time:
                        continue
                    v = traj['vs'][i]
                    if v<stop_sp_range:
                        move_stat.traj_id.append(n)
                        move_stat.index.append(i)
                        move_stat.time.append(traj['ts'][i])
                        move_stat.loc.append(traj['ps'][i])
                        break 
        return stop_stat,move_stat      
        
    focus_traj = focus_vehicles(traj,last_green_time,current_time)  
    stop_stat,move_stat = update_veh_stat(traj,focus_traj,current_time,current_signal)
    a1 = 0
    w1 = default_shock1_sp
    a2 = 0
    w2 = default_shock2_sp
    if len(stop_stat.time) == 0:
        w1 = 0
        w2 = 0
    if len(stop_stat.time)>1:
        t = np.array(stop_stat.time).reshape((-1, 1))
        l = np.array(stop_stat.loc)
        model = LinearRegression().fit(t, l)
        a1 = model.intercept_
        w1 = np.ndarray.item(model.coef_)
    if len(move_stat.time)>1:
        t = np.array(move_stat.time).reshape((-1, 1))
        l = np.array(move_stat.loc)
        model = LinearRegression().fit(t, l)
        a2 = model.intercept_
        w2 = np.ndarray.item(model.coef_)
        
    return a1,w1,a2,w2,stop_stat,move_stat 
    