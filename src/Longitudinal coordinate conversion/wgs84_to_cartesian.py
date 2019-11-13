from math import cos, radians, sin, sqrt
import numpy as np

def main(lat,lon,h,ref_lat,ref_lon):

    # Ellipsoid parameters: semi major axis in metres, reciprocal flattening.
    GRS80 = 6378137, 298.257222100882711
    WGS84 = 6378137, 298.257223563
    
    class Points:
        x = []
        y = []
        z = []
        
    def geodetic_to_geocentric(ellipsoid, latitude, longitude, height):
        """Return geocentric (Cartesian) Coordinates x, y, z corresponding to
        the geodetic coordinates given by latitude and longitude (in
        degrees) and height above ellipsoid. The ellipsoid must be
        specified by a pair (semi-major axis, reciprocal flattening).

        """
        φ = radians(latitude)
        λ = radians(longitude)
        sin_φ = sin(φ)
        a, rf = ellipsoid           # semi-major axis, reciprocal flattening
        e2 = 1 - (1 - 1 / rf) ** 2  # eccentricity squared
        n = a / sqrt(1 - e2 * sin_φ ** 2) # prime vertical radius
        r = (n + height) * cos(φ)   # perpendicular distance from z axis
        x = r * cos(λ)
        y = r * sin(λ)
        z = (n * (1 - e2) + height) * sin_φ
        return x, y, z

    
    # print(values_all[:])

    ref_point = geodetic_to_geocentric(WGS84, ref_lat, ref_lon, h)
    # print(ref_point)

    points = Points()
    points.x = np.zeros(len(lat))
    points.y = np.zeros(len(lat))
    points.z = np.zeros(len(lat))
    for i in range(len(lat)):
        points.x[i],points.y[i],points.z[i] = np.subtract(geodetic_to_geocentric(WGS84, lat[i], lon[i], h),ref_point)
        # print(i,converted_point)
        
    return points
    
    



