/*
 * Conversions.cpp
 *
 *  Created on: Oct 14, 2014
 *      Author: ivp
 */

// Officially, the ANSI standard does not include the math constant definitions, such as M_PI used below

#include "Conversions.h"

using namespace tmx::messages;

namespace tmx {
namespace utils {

#define DSRC_POLAR_RADIUS 6356752.314247833
#define DSRC_EQUATORIAL_RADIUS 6378137
double Conversions::ConvertMetersToMiles(double meters)
{
	return meters * Units::MILES_PER_METER;
}

double Conversions::ConvertMilesToMeters(double miles)
{
	return miles * Units::METERS_PER_MILE;
}

double Conversions::ConvertMetersPerSecToMilesPerHour(double mps)
{
	return mps * Units::MPH_PER_MPS;
}

double Conversions::ConvertDegreesToRadians(double degrees)
{
	return degrees * Units::RADIANS_PER_DEGREE;
}

double Conversions::ConvertRadiansToDegrees(double radians)
{
	return radians * Units::DEGREES_PER_RADIAN;
}

int Conversions::ConvertMetersPerSecToMPH(double mps)
{
	double mph = ConvertMetersPerSecToMilesPerHour(mps);

	int mph_int = round((float) mph);

	return mph_int;
}

double Conversions::DistanceMeters(double degreesLat1, double degreesLon1, double degreesLat2, double degreesLon2)
{
	// Find radius
	double earthRadius = 6371;
	double dLat = ConvertDegreesToRadians(degreesLat1) - ConvertDegreesToRadians(degreesLat2);
	double dLon = ConvertDegreesToRadians(degreesLon1) - ConvertDegreesToRadians(degreesLon2);
	double a = sin(dLat / 2) * sin(dLat / 2)
			+ cos(ConvertDegreesToRadians(degreesLat2))
			* cos(ConvertDegreesToRadians(degreesLat1))
			* sin(dLon / 2) * sin(dLon / 2);
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));
	double distance_km = earthRadius * c;
	double distance_m = distance_km * 1000.0;
	return distance_m;
}

double Conversions::DistanceMeters(WGS84Point point1, WGS84Point point2)
{
	return DistanceMeters(point1.Latitude, point1.Longitude, point2.Latitude, point2.Longitude);
}

double Conversions::GradeDegrees(WGS84Point point1, WGS84Point point2)
{
	double distance = DistanceMeters(point1, point2);

	double elevation_change_m = point2.Elevation - point1.Elevation;

	double riseOverRun = elevation_change_m/distance;

	double grade_Deg = ConvertRadiansToDegrees(atan(riseOverRun));

	return grade_Deg;

}

/// Return the bearing between two points in degrees from 0 - 360.
double Conversions::GetBearingDegrees(WGS84Point point1, WGS84Point point2)
{
	double lat1 = ConvertDegreesToRadians(point1.Latitude);
	double long1 = ConvertDegreesToRadians(point1.Longitude);
	double lat2 = ConvertDegreesToRadians(point2.Latitude);
	double long2 = ConvertDegreesToRadians(point2.Longitude);

	double deltaLong = long2 - long1;

	double y = sin(deltaLong) * cos(lat2);
	double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(deltaLong);
	double bearing = atan2(y, x);
	double bearingDegrees = ConvertRadiansToDegrees(bearing);

	if (bearingDegrees < 0)
		bearingDegrees += 360;

	return bearingDegrees;
}

double Conversions::NodeOffsetToLatitude(double baseLatitude, double totalYOffsetMeters)
{
	return baseLatitude + (totalYOffsetMeters * 360.0 / (2 * M_PI * DSRC_POLAR_RADIUS));
}
double Conversions::NodeOffsetToLongitude(double baseLongitude, double baseLatitude, double totalXOffsetMeters)
{
 return baseLongitude + (totalXOffsetMeters * 360.0 / (2 * M_PI * DSRC_EQUATORIAL_RADIUS * cos(baseLatitude * M_PI / 180)));
}
//WGS84Point Conversions::NodeOffsetToLatLong(double latOffset, double longOffset, WGS84Point nodeAnchor)
//{
//
//
//	double baseLatitude = nodeAnchor.Latitude / 10000000.0;
//	double baseLongitude = nodeAnchor.Longitude / 10000000.0;
//
//	double totalXOffset = 0.0;
//	double totalYOffset = 0.0;
//	double xOffset;
//	double yOffset;
//	/*The geometry of a lane is described by a list of nodes (always at least two) each with a
//	 * Northerly and Easterly offset (positive values) or Southerly and Westerly offsets (negative values).
//	 *  The offsets for the first node are relative to the intersections reference point that is given
//	 *  as a lat/long position, the offsets for all remaining nodes, after the first one, are relative
//	 *   to the previous node.  You should typically set you offset resolution to decimeter.
//	 *
//	 */
//	for (int i = 0; i < nodesLength; i++)
//	{
//		GetOffsets(nodes[i], &xOffset, &yOffset);
//		//std::cout << "xOffset " << xOffset << ", yOffset " << yOffset << std::endl;
//		totalXOffset += xOffset;
//		totalYOffset += yOffset;
//		points[i].Latitude = NodeOffsetToLatitude(baseLatitude,totalYOffset);
//		points[i].Longitude = NodeOffsetToLongitude(baseLongitude, totalXOffset);
//	}
//
//	return points;
//}

}} // namespace tmx::utils
