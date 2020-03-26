import numpy as np
import math

class Traj():
    ts = []
    vs = []
    ps = []

def sort_list(list1, list2): 
    zipped_pairs = zip(list2, list1) 
    z = [x for _, x in sorted(zipped_pairs)] 
    return z 

def main_func(road_points,traj_points,signal_loc,space_threshold):
    # sort the road points according to the distance to the signal location
    dist = np.zeros(len(road_points.x))
    for i in range(len(road_points.x)):
        dist[i] = math.sqrt((road_points.x[i]-signal_loc.x)**2+(road_points.y[i]-signal_loc.y)**2)
    road_points_sorted_x = sort_list(road_points.x,dist)
    road_points_sorted_y = sort_list(road_points.y,dist)
    
    # calculate cumulative distance and the line equation between each consecutive points
    cum_dist = np.zeros(len(road_points.x))
    eq = np.zeros((len(road_points.x),4)) # slope_x, slope_y, x0, y0
    for i in range(len(road_points_sorted_x)):
        if i==0:
            pre_dist = min(dist)
            dist = np.zeros(len(road_points.x)-1)
            cum_dist[i] = pre_dist
        else:
            pre_dist = cum_dist[i-2]
            dist[i-1] = math.sqrt((road_points_sorted_x[i-1]-road_points_sorted_x[i])**2+(road_points_sorted_y[i-1]-road_points_sorted_y[i])**2)
            cum_dist[i] = pre_dist + dist[i-1]
        eq[i,0] = road_points_sorted_x[i]-road_points_sorted_x[i-1]
        eq[i,1] = road_points_sorted_y[i]-road_points_sorted_y[i-1]
        eq[i,2] = road_points_sorted_x[i]
        eq[i,3] = road_points_sorted_y[i]
        
    # project gps points onto the longitudinal coordinate system
    traj_cum_dist = np.empty(len(traj_points.x)-1)
    traj_cum_dist[:] = np.nan
    index = []
    for k in range(len(traj_points.x)):
        # for each point, find the two closest road points
        d = np.zeros(len(road_points_sorted_x))
        for j in range(len(road_points_sorted_x)):
            d[j] = math.sqrt((traj_points.x[k]-road_points_sorted_x[j])**2+(traj_points.y[k]-road_points_sorted_y[j])**2)
        # indices of the 2 closest road points are d_index[0] and d_index[1]
        if min(d)>space_threshold:
            continue
        d_index = sorted(range(len(d)), key=lambda k: d[k])
        # find which one is head and which is tail
        head_index = min(d_index[0],d_index[1])
        # the vector between traj point k and head point (i-1)
        v_slope_x = traj_points.x[k]-road_points_sorted_x[head_index]
        v_slope_y = traj_points.y[k]-road_points_sorted_y[head_index]
        # vector projection of vector v on vector u (vector u connecting road points i and i-1)
        v = [v_slope_x,v_slope_y]
        u = [eq[head_index,0],eq[head_index,1]]
        proj = np.dot((np.dot(u,v)/(dist[head_index]**2)),u)
        # if projection is on the same direction, the point distance is added to cum_dist[head_index], otherwise we substract it
        traj_cum_dist[k] = cum_dist[head_index] + np.sign(proj[0]*u[0])*math.sqrt(proj[0]**2+proj[1]**2)
        index.append(k)
    
    traj_output = Traj()
    traj_output.id = [traj_points.id[i] for i in index]
    traj_output.ts = [traj_points.ts[i] for i in index]
    traj_output.vs = [traj_points.vs[i] for i in index]
    traj_output.ps = [traj_cum_dist[i] for i in index]
        
    return traj_output