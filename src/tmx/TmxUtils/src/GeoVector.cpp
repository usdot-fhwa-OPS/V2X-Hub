/*
 * GeoVector.cpp
 *
 *  Created on: Oct 6, 2016
 *      Author: ivp
 */

// Officially, the ANSI standard does not include the math constant definitions, such as M_PI used below
#ifdef __STRICT_ANSI__
#undef __STRICT_ANSI__
#include <cmath>
#define __STRICT_ANSI__
#else
#include <cmath>
#endif

#include "GeoVector.h"

namespace tmx {
namespace utils {

/*
 * initialization of statics
 */
const double GeoVector::_earthRadiusInKM = 6371.0;

/*
 * Constructors
 */

GeoVector::GeoVector(double x, double y, double z) : _x(x), _y(y), _z(z)
{
}

/*
 * Convert WGS84Point (lat/long) to NVector
 *
 * return GeoVector
 */
GeoVector GeoVector::WGS84PointToNVector(WGS84Point point)
{
	GeoVector vec;
	double lat;
	double lon;
	//convert lat/lon to radians
	lat = point.Latitude * M_PI / 180.0;
	lon = point.Longitude * M_PI / 180.0;
	//create right handed vector x -> 0°E,0°N; y -> 90°E,0°N, z -> 90°N
	vec._x = cos(lat) * cos(lon);
	vec._y = cos(lat) * sin(lon);
	vec._z = sin(lat);
	return vec;
}

/*
 * Convert NVector to WGS84Point (lat/long)
 *
 * return WGS84Point
 */
WGS84Point GeoVector::NVectorToWGS84Point(GeoVector vec)
{
	WGS84Point point;
	point.Latitude = atan2(vec._z, sqrt((vec._x * vec._x) + (vec._y * vec._y))) * 180.0 / M_PI;
	point.Longitude = atan2(vec._y, vec._x) * 180.0 / M_PI;
	return point;
}

/*
 * Calculate vector dot product
 *
 * return double
 */
double GeoVector::Dot(GeoVector vec1, GeoVector vec2)
{
	return (vec1._x * vec2._x) + (vec1._y * vec2._y) + (vec1._z * vec2._z);
}

/*
 * Calculate vector cross product
 *
 * return GeoVector
 */
GeoVector GeoVector::Cross(GeoVector vec1, GeoVector vec2)
{
	GeoVector vec;
	vec._x = (vec1._y * vec2._z) - (vec1._z * vec2._y);
	vec._y = (vec1._z * vec2._x) - (vec1._x * vec2._z);
	vec._z = (vec1._x * vec2._y) - (vec1._y * vec2._x);
	return vec;
}

/*
 * Calculate magnitude or norm of vector
 *
 * return double
 */
double GeoVector::Length(GeoVector vec)
{
	return sqrt((vec._x * vec._x) + (vec._y * vec._y) + (vec._z * vec._z));
}


/*
 * Calculate vec1 + vec2
 *
 * return GeoVector
 */
GeoVector GeoVector::Plus(GeoVector vec1, GeoVector vec2)
{
	GeoVector vec;
	vec._x = vec1._x + vec2._x;
	vec._y = vec1._y + vec2._y;
	vec._z = vec1._z + vec2._z;
	return vec;
}

/*
 * Calculate vec1 - vec2
 *
 * return GeoVector
 */
GeoVector GeoVector::Minus(GeoVector vec1, GeoVector vec2)
{
	GeoVector vec;
	vec._x = vec1._x - vec2._x;
	vec._y = vec1._y - vec2._y;
	vec._z = vec1._z - vec2._z;
	return vec;
}

/*
 * Normalize vector to its unit vector
 *
 * return GeoVector
 */
GeoVector GeoVector::Unit(GeoVector vec)
{
	GeoVector nvec;
	double norm = Length(vec);
	if (norm == 1)
		return vec;
	if (norm == 0)
		return vec;
	nvec._x = vec._x / norm;
	nvec._y = vec._y / norm;
	nvec._z = vec._z / norm;
	return nvec;
}

/*
 * Multiply vector by a value
 *
 * return GeoVector
 */
GeoVector GeoVector::Times(GeoVector vec, double value)
{
	GeoVector rvec;
	rvec._x = vec._x * value;
	rvec._y = vec._y * value;
	rvec._z = vec._z * value;
	return rvec;
}



/*
 * Calculate angle between vec1 and vec2 in radians (-pi to pi)
 * If signVec is not supplied angle is unsigned.
 * If signVec is supplied (must be out of the plane of vec1 and vec2) then sign is positive if vec1
 * is clockwise looking along signVec, otherwise sign is negative.
 *
 * return radians (-pi to pi) as double
 *
 */
double GeoVector::AngleBetweenInRadians(GeoVector vec1, GeoVector vec2, GeoVector signVec)
{
	double angle = atan2(Length(Cross(vec1, vec2)), Dot(vec1, vec2));
	if (signVec._x == 0.0 && signVec._y == 0.0 && signVec._z == 0.0)
	{
		// if signVec is invalid return unsigned angle
		return angle;
	}
	//determine sign
	if (Dot(Cross(vec1, vec2),signVec) < 0.0)
	{
		angle = -angle;
	}
	return angle;
}

/*
 * Calculate distance between two WGS84Point in meters
 *
 * return meters as double
 */
double GeoVector::DistanceInMeters(WGS84Point point1, WGS84Point point2)
{
	GeoVector vec1 = WGS84PointToNVector(point1);
	GeoVector vec2 = WGS84PointToNVector(point2);
	return DistanceInMeters(vec1, vec2);
}

/*
 * Calculate distance between two GeoVector in meters
 *
 * return meters as double
 */
double GeoVector::DistanceInMeters(GeoVector vec1, GeoVector vec2)
{
	double angle = AngleBetweenInRadians(vec1, vec2);
	return angle * _earthRadiusInKM * 1000.0;
}

/*
 * Calculate great circle given a point and a bearing (degrees 0 to 360)
 *
 * return GeoVector
 */
GeoVector GeoVector::GreatCircle(GeoVector vec, double bearing)
{
	GeoVector gc;
	double lat;
	double lon;
	double bear;
	WGS84Point point;
	point = NVectorToWGS84Point(vec);
	//cout << "Point: " << point.Latitude << "," << point.Longitude << "\n";
	//convert to radians
	lat = point.Latitude * M_PI / 180.0;
	lon = point.Longitude * M_PI / 180.0;
	bear = bearing * M_PI / 180.0;
	gc._x = (sin(lon) * cos(bear)) - (sin(lat) * cos(lon) * sin(bear));
	gc._y = (cos(lon) * -1.0 * cos(bear)) - (sin(lat) * sin(lon) * sin(bear));
	gc._z = cos(lat) * sin(bear);

	return gc;
}


/*
 * Calculate initial bearing from point1 to point2 in degrees from north (0 to 360)
 *
 * return degrees from north (0 to 360) as double
 */
double GeoVector::BearingInDegrees(WGS84Point point1, WGS84Point point2)
{
	GeoVector vec1 = WGS84PointToNVector(point1);
	GeoVector vec2 = WGS84PointToNVector(point2);
	GeoVector northPole(0, 0, 1);
	GeoVector c1; //great circle through point1 and point2 surface normal
	GeoVector c2; //great circle through point1 and north pole surface normal
	double bearing;

	// calculate great circle surface normals
	c1 = GeoVector::Cross(vec1, vec2);
	c2 = GeoVector::Cross(vec1, northPole);

	//signed bearing in degrees (-180 to 180)
	bearing = AngleBetweenInRadians(c1, c2, vec1) * 180.0 / M_PI;
	//return normalized bearing (0 to 360)
	if (bearing < 0.0)
		bearing += 360.0;

	return bearing;
}

/*
 * Calculate point of intersection of two paths.
 *
 * If c1 and c2 are great circles through start and end points then candidate intersections are c1 × c2 and c2 × c1.
 * Choose closer intersection.
 *
 * return WGS84Point
 */
WGS84Point GeoVector::Intersection(WGS84Point path1P1, WGS84Point path1P2, WGS84Point path2P1, WGS84Point path2P2)
{
	GeoVector p1v1 = WGS84PointToNVector(path1P1);
	GeoVector p1v2 = WGS84PointToNVector(path1P2);
	GeoVector p2v1 = WGS84PointToNVector(path2P1);
	GeoVector p2v2 = WGS84PointToNVector(path2P2);
	GeoVector c1; //great circle through path1P1 and path1P2 surface normal
	GeoVector c2; //great circle through path2P1 and path2P2 surface normal
	GeoVector i1; // intersection 1
	GeoVector i2; // intersection 2
	double sum1, sum2;

	// calculate great circle surface normals
	c1 = Cross(p1v1, p1v2);
	c2 = Cross(p2v1, p2v2);

	// get both intersections
	i1 = Cross(c1, c2);
	i2 = Cross(c2, c1);

	//calculate sum of distances from all points to each intersection, choose closest
	sum1 = DistanceInMeters(p1v1, i1) + DistanceInMeters(p1v2, i1) +
			DistanceInMeters(p2v1, i1) + DistanceInMeters(p2v2, i1);
	sum2 = DistanceInMeters(p1v1, i2) + DistanceInMeters(p1v2, i2) +
			DistanceInMeters(p2v1, i2) + DistanceInMeters(p2v2, i2);

	if (sum1 < sum2)
		return NVectorToWGS84Point(i1);

	return NVectorToWGS84Point(i2);
}

/*
 * Calculate point of intersection of two paths.
 *
 * If c1 and c2 are great circles through start and end points then candidate intersections are c1 × c2 and c2 × c1.
 * Choose closer intersection.
 *
 * return WGS84Point
 */

WGS84Point GeoVector::Intersection(WGS84Point path1P1, double path1Bearing, WGS84Point path2P1, double path2Bearing)
{
	GeoVector p1v1 = WGS84PointToNVector(path1P1);
	GeoVector p2v1 = WGS84PointToNVector(path2P1);
	GeoVector c1; //great circle through path1P1 and path1P2 surface normal
	GeoVector c2; //great circle through path2P1 and path2P2 surface normal
	GeoVector i1; // intersection 1
	GeoVector i2; // intersection 2
	double sum1, sum2;

	// calculate great circle surface normals
	c1 = GreatCircle(p1v1, path1Bearing);
	c2 = GreatCircle(p2v1, path2Bearing);

	// get both intersections
	i1 = Cross(c1, c2);
	i2 = Cross(c2, c1);

	//calculate sum of distances from all points to each intersection, choose closest
	sum1 = DistanceInMeters(p1v1, i1) + DistanceInMeters(p2v1, i1);
	sum2 = DistanceInMeters(p1v1, i2) + DistanceInMeters(p2v1, i2);

	if (sum1 < sum2)
		return NVectorToWGS84Point(i1);

	return NVectorToWGS84Point(i2);
}

/*
 *  Calculate new position given starting point with bearing and distance traveled in meters
 *
 *  return WGS84Point
 */
WGS84Point GeoVector::DestinationPoint(WGS84Point point, double bearing, double distanceTraveledInMeters)
{
	GeoVector n1;
	double angle;
	double earthRadiusInMeters = _earthRadiusInKM * 1000.0;
	double b;
	GeoVector northPole(0, 0, 1);
	GeoVector de;  // direction east
	GeoVector dn;  // direction north
	GeoVector deSin;
	GeoVector dnCos;
	GeoVector d;  // direction vector at n1 (C x n1 where C = great circle)
	GeoVector x;  // component of n2 parallel to n1
	GeoVector y;  // component of n2 perpendicular to n1
	GeoVector n2;

	n1 = WGS84PointToNVector(point);
	angle = distanceTraveledInMeters / earthRadiusInMeters;  // angle in radians
	b = bearing * M_PI / 180.0;  // bearing in radians

	de = Cross(northPole, n1);
	de = Unit(de);
	dn = Cross(n1, de);
	deSin = Times(de, sin(b));
	dnCos = Times(dn, cos(b));
	d = Plus(dnCos, deSin);
	x = Times(n1, cos(angle));
	y = Times(d, sin(angle));
	n2 = Plus(x, y);
	// you have got to be kidding me
	return NVectorToWGS84Point(n2);
}

/*
 * Calculate cross track distance, the distance in meters from a point to the great circle defined
 * by a path start point and end point, distance is signed (negative to left of path, positive to right of path)
 *
 * return meters as double
 */
double GeoVector::CrossTrackDistanceInMeters(WGS84Point point, WGS84Point pathP1, WGS84Point pathP2)
{
	GeoVector vec1 = WGS84PointToNVector(point);
	GeoVector pv1 = WGS84PointToNVector(pathP1);
	GeoVector pv2 = WGS84PointToNVector(pathP2);
	GeoVector c1; //great circle through pathP1 and pathP2 surface normal
	double angle;

	// calculate great circle surface normal
	c1 = Cross(pv1, pv2);

	// calculate angle between surface and point
	angle = AngleBetweenInRadians(c1, vec1) - (M_PI / 2);

	//return distance in meters
	return angle * _earthRadiusInKM * 1000.0;
}


/*
 * Calculate cross track distance, the distance in meters from a point to the great circle defined
 * by a path start point and bearing, distance is signed (negative to left of path, positive to right of path)
 *
 * return meters as double
 */
double GeoVector::CrossTrackDistanceInMeters(WGS84Point point, WGS84Point pathP1, double pathBearing)
{
	GeoVector vec1 = WGS84PointToNVector(point);
	GeoVector pv1 = WGS84PointToNVector(pathP1);
	GeoVector c1; //great circle from pathP1 using bearing
	double angle;

	// calculate great circle surface normal
	c1 = GreatCircle(pv1, pathBearing);

	// calculate angle between surface and point
	angle = AngleBetweenInRadians(c1, vec1) - (M_PI / 2);

	//return distance in meters
	return angle * _earthRadiusInKM * 1000.0;
}

/*
 * Calculate the signed angle from path1 to path2 (-180 to 180)
 * Paths are defined by their GPS coordinate pairs
 *
 * return degrees from north (-180 to 180) as double
 */
double GeoVector::AngleBetweenPathsInDegrees(WGS84Point path1P1, WGS84Point path1P2, WGS84Point path2P1, WGS84Point path2P2)
{
	GeoVector p1v1 = WGS84PointToNVector(path1P1);
	GeoVector p1v2 = WGS84PointToNVector(path1P2);
	GeoVector p2v1 = WGS84PointToNVector(path2P1);
	GeoVector p2v2 = WGS84PointToNVector(path2P2);
	GeoVector c1; //great circle through path1P1 and path1P2 surface normal
	GeoVector c2; //great circle through path2P1 and path2P2 surface normal
	double angle;

	// calculate great circle surface normals
	c1 = Cross(p1v1, p1v2);
	c2 = Cross(p2v1, p2v2);

	// calculate angle between surface normals using vector to first point as sign vector
	angle = AngleBetweenInRadians(c2, c1, p1v1) * 180.0 / M_PI;

	//cout << "GC1: " << c1._x << "," << c1._y << "," << c1._z << "\n";
	return angle;
}

/*
 * Calculate the signed angle from path1 to path2 (-180 to 180)
 * Path1 is defined by its GPS start point and bearing, path2 is a GPS coordinate pair
 *
 * return degrees from north (-180 to 180) as double
 */
double GeoVector::AngleBetweenPathsInDegrees(WGS84Point path1P1, double path1Bearing, WGS84Point path2P1, WGS84Point path2P2)
{
	GeoVector p1v1 = WGS84PointToNVector(path1P1);
	GeoVector p2v1 = WGS84PointToNVector(path2P1);
	GeoVector p2v2 = WGS84PointToNVector(path2P2);
	GeoVector c1; //great circle through path1P1 and path1Bearing
	GeoVector c2; //great circle through path2P1 and path2P2 surface normal
	double angle;

	// calculate great circle surface normals
	c1 = GreatCircle(p1v1, path1Bearing);
	c2 = Cross(p2v1, p2v2);

	// calculate angle between surface normals using vector to first point as sign vector
	angle = AngleBetweenInRadians(c2, c1, p1v1) * 180.0 / M_PI;

	//cout << "GC1(B): " << c1._x << "," << c1._y << "," << c1._z << "  B: " << path1Bearing << "\n";
	return angle;
}

/*
 * Calculate the midpoint between two GPS coordinate points
 *
 * return WGS84Point
 */
WGS84Point GeoVector::MidpointBetween(WGS84Point point1, WGS84Point point2)
{
	GeoVector vec;
	GeoVector vec1 = WGS84PointToNVector(point1);
	GeoVector vec2 = WGS84PointToNVector(point2);
	vec = Plus(vec1, vec2);
	vec = Unit(vec);
	return NVectorToWGS84Point(vec);
}

/*
 * Test if point is between two points of a path segment. If point is not on path return true if
 * point is between perpendiculars from path segment points.
 *
 * return bool
 */
bool GeoVector::IsBetween(WGS84Point point, WGS84Point pathP1, WGS84Point pathP2)
{
	GeoVector vec1 = WGS84PointToNVector(point);
	GeoVector pv1 = WGS84PointToNVector(pathP1);
	GeoVector pv2 = WGS84PointToNVector(pathP2);
	GeoVector d10, d12, d20, d21;
	double extent1, extent2;
	d10 = Minus(vec1, pv1);
	d12 = Minus(pv2, pv1);
	d20 = Minus(vec1, pv2);
	d21 = Minus(pv1, pv2);
	extent1 = Dot(d10, d12);
	extent2 = Dot(d20, d21);
	if (extent1 >= 0.0 && extent2 >= 0.0)
		return true;
	return false;
}

/*
 * Return the nearest point on a path segment if point argument is between segment endpoints. If point
 * argument is not between segment endpoints return nearest endpoint.
 *
 * return WGS84Point
 */
WGS84Point GeoVector::NearestPointOnSegment(WGS84Point point, WGS84Point pathP1, WGS84Point pathP2)
{
	GeoVector vec1 = WGS84PointToNVector(point);
	GeoVector pv1 = WGS84PointToNVector(pathP1);
	GeoVector pv2 = WGS84PointToNVector(pathP2);
	GeoVector c1, c2, n;
	double d1, d2;
	if (IsBetween(point, pathP1, pathP2))
	{
		c1 = Cross(pv1, pv2); //surface normal of great circle through pathP1 and pathP2
		c2 = Cross(vec1, c1); //surface normal of great circle through point normal to c1
		n = Cross(c1, c2);
	}
	else
	{
		d1 = DistanceInMeters(vec1, pv1);
		d2 = DistanceInMeters(vec1, pv2);
		if (d1 < d2)
			return pathP1;
		else
			return pathP2;
	}
	return NVectorToWGS84Point(n);
}

/*
 * Determine if a point is enclosed by a polygon
 * Polygon is defined by a vector of vertices, first vertex is not duplicated
 *
 * return bool
 */
bool GeoVector::IsEnclosedBy(WGS84Point point, std::vector<WGS84Point> &polygon)
{
	WGS84Point p1;
	WGS84Point p2;
	double angle = 0.0;
	bool isFirstPoint = true;
	//add angles between point and vertices
	for (auto it = polygon.begin() ; it != polygon.end(); ++it)
	{
		p1 = p2;
		p2 = *it;
		if (!isFirstPoint)
			angle += AngleBetweenPathsInDegrees(point, p1, point, p2);
		else
			isFirstPoint = false;
	}
	//add angle between last vertex and first vertex
	angle += AngleBetweenPathsInDegrees(point, polygon.back(), point, polygon.front());
	if (fabs(angle) >= 360.0)
		return true;
	else
		return false;
}


}
} // namespace tmx::utils
