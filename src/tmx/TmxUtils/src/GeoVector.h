/*
 * GeoVector.h
 *
 *  Created on: Oct 6, 2016
 *      Author: ivp
 */

#ifndef GEOVECTOR_H_
#define GEOVECTOR_H_

#include "WGS84Point.h"
#include <vector>

namespace tmx {
namespace utils {

/*
 * GeoVector is a 3 dimensional vector manipulation class that
 * implements a vector based method for working with
 * latitude/longitude based calculations (spherical earth model).
 *
 * NVector is a type of GeoVector representing a point on the earths surface
 *   as the surface normal vector at that point.
 */

class GeoVector
{
private:

	double _x = 0.0;
	double _y = 0.0;
	double _z = 0.0;
	static const double _earthRadiusInKM;

public:
	GeoVector(double x = 0, double y = 0, double z = 0);

	//vector methods
	static GeoVector WGS84PointToNVector(WGS84Point point);
	static WGS84Point NVectorToWGS84Point(GeoVector vec);
	static double Dot(GeoVector vec1, GeoVector vec2);
	static GeoVector Cross(GeoVector vec1, GeoVector vec2);
	static double Length(GeoVector vec);
	static double AngleBetweenInRadians(GeoVector vec1, GeoVector vec2, GeoVector signVec = GeoVector(0.0, 0.0, 0.0));
	static GeoVector GreatCircle(GeoVector vec, double bearing);
	static GeoVector Plus(GeoVector vec1, GeoVector vec2);
	static GeoVector Minus(GeoVector vec1, GeoVector vec2);
	static GeoVector Unit(GeoVector vec);
	static GeoVector Times(GeoVector vec, double value);
	static double DistanceInMeters(GeoVector vec1, GeoVector vec2);

	//GPS coordinate interface
	static double DistanceInMeters(WGS84Point point1, WGS84Point point2);
	static double BearingInDegrees(WGS84Point point1, WGS84Point point2);
	static WGS84Point Intersection(WGS84Point path1P1, WGS84Point path1P2, WGS84Point path2P1, WGS84Point path2P2);
	static WGS84Point Intersection(WGS84Point path1P1, double path1Bearing, WGS84Point path2P1, double path2Bearing);
	static WGS84Point DestinationPoint(WGS84Point point, double bearing, double distanceTraveledInMeters);
	static double CrossTrackDistanceInMeters(WGS84Point point, WGS84Point pathP1, WGS84Point pathP2);
	static double CrossTrackDistanceInMeters(WGS84Point point, WGS84Point pathP1, double pathBearing);
	static double AngleBetweenPathsInDegrees(WGS84Point path1P1, WGS84Point path1P2, WGS84Point path2P1, WGS84Point path2P2);
	static double AngleBetweenPathsInDegrees(WGS84Point path1P1, double path1Bearing, WGS84Point path2P1, WGS84Point path2P2);
	static WGS84Point MidpointBetween(WGS84Point point1, WGS84Point point2);
	static bool IsBetween(WGS84Point point, WGS84Point pathP1, WGS84Point pathP2);
	static WGS84Point NearestPointOnSegment(WGS84Point point, WGS84Point pathP1, WGS84Point pathP2);
	static bool IsEnclosedBy(WGS84Point point, std::vector<WGS84Point> &polygon);

};

}} // namespace tmx::utils

#endif /* GEOVECTOR_H_ */
