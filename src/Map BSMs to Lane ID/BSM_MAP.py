import json
import os
import pandas as pd
import numpy as np
import geopy.distance
import csv

# if the minimum distance between each BSM and MAP GPS coordinates is greater than dist_threshold, the BSM coordinates are considered out of boundary
dist_threshold = 5 # meters

MAP_address = r'C:\Users\ghiasia\Documents\Projects\18 - 327 OpsIV V2X Hub\Metrics\Code\MAP\ISD_9709_child_r3 - json_converted.geojson'
BSM_address = r'C:\Users\ghiasia\Documents\Data\BSM_logs\12-20-19'
output_address = r'C:\Users\ghiasia\Documents\Projects\18 - 327 OpsIV V2X Hub\Metrics\Converted BSMs'

class MAP_GPS():
    lat = []
    lon = []
    lane = []

with open(MAP_address) as f:
    data = json.load(f)

# list all MAP GPS points
num_lanes = len(data['lanes']['features'])
map_gps = MAP_GPS()
for l in range(num_lanes):
    num_points = len(data['lanes']['features'][l]['properties']['elevation'])
    for i in range(num_points):
        map_gps.lon.append(data['lanes']['features'][l]['properties']['elevation'][i]['latlon']['lon'])
        map_gps.lat.append(data['lanes']['features'][l]['properties']['elevation'][i]['latlon']['lat'])
        map_gps.lane.append(int(data['lanes']['features'][l]['properties']['laneNumber']))

# reading BSM files
# read the header from the first file
# file_address = BSM_address + '\\' + os.listdir(BSM_address)[0]
# with open(file_address, "rt", encoding='utf8') as f:
#     reader = csv.reader(f)
#     header = next(reader)
#     header.append('Lane #')
    
check_header = True
for file_name in os.listdir(BSM_address):
    if file_name.endswith('.csv'):
        file_address = BSM_address + '\\' + file_name
        if check_header == True:
            with open(file_address, "rt", encoding='utf8') as f:
                reader = csv.reader(f)
                header = next(reader)
                header.append('Lane #')
            check_header = False
        with open(file_address) as csv_file:
            csv_reader = pd.read_csv(csv_file, skiprows=1)
            values_all = csv_reader.values
            bsm_num = len(values_all)
            lane_nums = np.zeros(num_points)
            lats = values_all[:,4]/10000000
            lons = values_all[:,5]/10000000
            bsm_lanes = np.empty(bsm_num)
            # find the distance between all bsm and map gps points. This code could be written in a more efficient way!
            for i in range(bsm_num):
                min_dist = dist_threshold
                temp_lane = 0
                for j in range(len(map_gps.lat)-1):
                    temp_dist = geopy.distance.VincentyDistance([lats[i],lons[i]],[map_gps.lat[j],map_gps.lon[j]]).m
                    if temp_dist < min_dist:
                        min_dist = temp_dist
                        temp_lane = map_gps.lane[j]
                bsm_lanes[i] = temp_lane
            values_all = np.array(values_all)
            fill_empty = np.full([len(values_all),len(header)-values_all.shape[1]-1], 'nan')
            values_all = np.column_stack((values_all,fill_empty))
            values_all = np.column_stack((values_all,bsm_lanes))
        output_file_address = output_address + '\\' + file_name
        with open(output_file_address, "w", newline='') as fp:
            writer = csv.writer(fp)
            writer.writerow(header)
            for row in range(len(values_all)-1):
                writer.writerow(values_all[row])