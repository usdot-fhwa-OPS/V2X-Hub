import numpy as np
from shapely.geometry import Point
from shapely.geometry import LineString

point = Point(0.2, 0.5)
line = LineString([(0, 1), (1, 1)])

x = np.array(point.coords[0])

u = np.array(line.coords[0])
v = np.array(line.coords[len(line.coords)-1])

n = v - u
n /= np.linalg.norm(n, 2)

P = u + n*np.dot(x - u, n)
print(P) #0.2 1.