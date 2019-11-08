# trajectory construction module
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def trajectory(input_address):

    class Trajectory:
        id = []
        vtype = []
        vlength = []
        ts = []
        ps = []
        vs = []
        ac = []
        link = []
        lane = []

    def read_csv(input_address):
        with open(input_address) as csv_file:
            csv_reader = pd.read_csv(csv_file)
            traj_raw = csv_reader.values
                
        return traj_raw
                
    def construct(traj_raw):
        i = 0
        traj = Trajectory()
        
        traj.link = int(traj_raw[0,4])
        traj.lane = int(traj_raw[0,5])
        while i<len(traj_raw):
            rec_num = int(traj_raw[i,3])
            traj.id.append(int(traj_raw[i,0]))
            traj.vtype.append(traj_raw[i,1])
            traj.vlength.append(traj_raw[i,9])
            traj.ts.append(traj_raw[i:i+rec_num-1,2])
            traj.ps.append(traj_raw[i:i+rec_num-1,6])
            traj.vs.append(traj_raw[i:i+rec_num-1,7])
            traj.ac.append(traj_raw[i:i+rec_num-1,8])
            i += rec_num          
        
        return traj

    def plot_traj(traj):
        plt.figure(figsize=(4,4))
        for i in range(len(traj.id)):
            if traj.vtype[i]==0:
                mark_color = 'b.'
            else:
                mark_color = 'r.'           
            plt.plot(traj.ts[i],traj.ps[i],mark_color,ms=0.5)
        plt.show(block = False)
        
    # input_address = r'C:\Users\ghiasia\Documents\Projects\18 - 327 OpsIV V2X Hub\Metrics\Code\Temp Data\traj_EB_l1.csv'
    traj_raw = read_csv(input_address)
    traj = construct(traj_raw)
    # plot_traj(traj)
    
    return traj
