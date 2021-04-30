import numpy as np
import utm

def main(lat,lon):
    
    class Points:
        x = []
        y = []

    points = Points()
    points.x = np.zeros(len(lat))
    points.y = np.zeros(len(lat))
    points.z = np.zeros(len(lat))
    for i in range(len(lat)):
        u = utm.from_latlon(lat[i], lon[i])
        points.x[i] = u[0]
        points.y[i] = u[1]
    return points
    