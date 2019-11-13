import os
import wgs84_to_cartesian
from math import sqrt
import numpy as np
import pandas as pd
import csv

## the road points are assumed to be in order in the csv file, but the first one is the intersection reference point. 
## dist variable is the distance between each pair of consecutive road points 
## eq variable is the equation between each pair of consecutive road points. columns: m,x0,y0 (y=y0+m(x-x0))
## we find the two closest road points to each vehicle xy points. indices of the 2 closest road points are d_index[0] and d_index[1]
## for each vehicle xy points, we project it to line connecting the two above-mentioned road points (using eq[i,i+1,:])
## after projecting, the cumulative distance of the vehicle point is an interpolation of cumulative distances of the two road points
## the output is another column that shows the cumulative distance of the vehicle 

mydir = os.getcwd()
# first record in input_raod file is the intersection reference point. the other points should be put sequentially
input_road = mydir + '\Input_road_gps_points.csv'
input_traj = mydir + '\Traj.csv'
output_traj = mydir + '\Traj_new.csv'

# Northen Virginia geodetic height of the reference point
# h = -51
h = float(input("Enter Geodetic height of the region: "))

with open(input_road) as csv_file:
    csv_reader = pd.read_csv(csv_file,header=None)
    csv_reader['dist'] = 0
    values_road = csv_reader.values
lat_road = np.zeros(len(values_road))
lon_road = np.zeros(len(values_road))
for i in range(len(values_road)):
    lat_road[i] = values_road[i][0]
    lon_road[i] = values_road[i][1]
road_points = wgs84_to_cartesian.main(lat_road[1:],lon_road[1:],h,lat_road[0],lon_road[0])

with open(input_traj) as csv_file:
    csv_reader = pd.read_csv(csv_file)
    values_traj = csv_reader.values
    header_text = pd.read_csv(input_traj, nrows=1).columns.tolist()
    # header_text = pd.read_csv(input_traj, index_col=0, nrows=0).columns.tolist()
lat_traj = np.zeros(len(values_traj))
lon_traj = np.zeros(len(values_traj))
for i in range(len(values_traj)):
    lat_traj[i] = values_traj[i][9]
    lon_traj[i] = values_traj[i][10]
traj_points = wgs84_to_cartesian.main(lat_traj,lon_traj,h,lat_road[0],lon_road[0])    

# calculate cumulative distance, the line equation between each consecutive points, and space threshold
dist = np.zeros(len(road_points.x)-1)
cum_dist = np.zeros(len(road_points.x)-1)
eq = np.zeros((len(road_points.x)-1,4)) # slope_x, slope_y, x0, y0
# if the distance between traj points and road points is greater than the space threshold, then the traj is out of road bounds
# in this code, space threshold is set to the maximum distance between road points
space_threshold = 0
for i in range(1,len(road_points.x)):
    # ignore z
    if i==1:
        pre_dist = 0
    else:
        pre_dist = cum_dist[i-2]
    dist[i-1] = sqrt((road_points.x[i-1]-road_points.x[i])**2+(road_points.y[i-1]-road_points.y[i])**2)
    cum_dist[i-1] = pre_dist + dist[i-1]
    space_threshold = max(space_threshold,dist[i-1])
    eq[i-1,0] = road_points.x[i]-road_points.x[i-1]
    eq[i-1,1] = road_points.y[i]-road_points.y[i-1]
    eq[i-1,2] = road_points.x[i-1]
    eq[i-1,3] = road_points.y[i-1]
    
# project gps points onto the longitudinal coordinate system
traj_cum_dist = np.zeros(len(traj_points.x)-1)
n,m = values_traj.shape
dist_col = np.zeros((n,1))
values_traj_new = np.hstack((values_traj,dist_col))
for k in range(len(traj_points.x)):
    # for each point, find the two closest road points
    d = np.zeros(len(road_points.x))
    for j in range(len(road_points.x)):
        d[j] = sqrt((traj_points.x[k]-road_points.x[j])**2+(traj_points.y[k]-road_points.y[j])**2)
    # indices of the 2 closest road points are d_index[0] and d_index[1]
    if min(d)>space_threshold:
        continue
    d_index = sorted(range(len(d)), key=lambda k: d[k])
    # find which one is head and which is tail
    head_index = min(d_index[0],d_index[1])
    # the vector between traj point k and head point (i-1) >> vector v
    v_slope_x = traj_points.x[k]-road_points.x[head_index]
    v_slope_y = traj_points.y[k]-road_points.y[head_index]
    # vector projection of vector v on vector u (vector u connecting road points i and i-1)
    # https://www.ck12.org/book/CK-12-College-Precalculus/section/9.6/
    v = [v_slope_x,v_slope_y]
    u = [eq[head_index,0],eq[head_index,1]]
    proj = np.dot((np.dot(u,v)/(dist[head_index]**2)),u)
    # if projection is on the same direction, the point distance is added to cum_dist[head_index], otherwise we substract it
    traj_cum_dist[k] = cum_dist[head_index] + np.sign(proj[0]*u[0])*sqrt(proj[0]**2+proj[1]**2)
    values_traj_new[k,-1] = traj_cum_dist[k]

# write values_traj_new to output_traj
index = np.where(values_traj_new[:,-1] > 0)[0]
header_text.append('cum distance')
pd.DataFrame(values_traj_new[index,:]).to_csv(output_traj, index=False,header = header_text)