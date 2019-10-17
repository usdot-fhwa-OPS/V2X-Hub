# Shockwave estimation module
import numpy as np
from sklearn.linear_model import LinearRegression

# Parameters
# default_shock1_sp = -4 # ft/s, this depends on traffic demand
# default_shock2_sp = -35 # ft/s
# stop_sp_range = 3.3 # ft/s

def shockwave(traj,last_green_time,current_time,current_signal,default_shock1_sp,default_shock2_sp,stop_sp_range):
    
    class Changing_Stat:
        traj_id = []
        index = []
        time = []
        loc = []
    
    def focus_vehicles(traj,last_green_time,current_time):
        # sorting is not necessary as some vehicles may take over in the region of interest.
        focus_traj = []
        for n in range(len(traj.id)):
            if traj.vtype[n]==1 and traj.ts[n][0]<current_time and traj.ts[n][-1]>last_green_time:
                focus_traj.append(n)
        return focus_traj
    def update_veh_stat(traj,focus_traj,current_time,current_signal):
        stop_stat = Changing_Stat()
        move_stat = Changing_Stat()     
        for n in focus_traj:
            for i in range(len(traj.vs[n])):
                if traj.ts[n][i]>current_time:
                    break
                v = traj.vs[n][i]
                if v<stop_sp_range:
                    stop_stat.traj_id.append(n)
                    stop_stat.index.append(i)
                    stop_stat.time.append(traj.ts[n][i])
                    stop_stat.loc.append(traj.ps[n][i])
                    break 
        if current_signal == 'GREEN': 
            for n in focus_traj:
                for i in reversed(range(len(traj.vs[n]))):
                    if traj.ts[n][i]>current_time:
                        continue
                    v = traj.vs[n][i]
                    if v<stop_sp_range:
                        move_stat.traj_id.append(n)
                        move_stat.index.append(i)
                        move_stat.time.append(traj.ts[n][i])
                        move_stat.loc.append(traj.ps[n][i])
                        break 
        return stop_stat,move_stat      
        
    focus_traj = focus_vehicles(traj,last_green_time,current_time)  
    stop_stat,move_stat = update_veh_stat(traj,focus_traj,current_time,current_signal)
    a1 = 0
    w1 = default_shock1_sp
    a2 = 0
    w2 = default_shock2_sp  
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
    

        
            