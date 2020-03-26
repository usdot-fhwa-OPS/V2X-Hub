/*
 * Conversions.cpp
 *
 *  Created on: Oct 14, 2014
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

#include "Conversions.h"

using namespace tmx::utils;

double Conversions::ConvertMetersToMiles(double meters)
{
	double miles = meters / 1609.344;
	return miles;
}

double Conversions::ConvertMilesToMeters(double miles)
{
	double meters = miles * 1609.344;
	return meters;
}

double Conversions::ConvertMetersPerSecToMilesPerHour(double mps)
{
	double mph = mps * 2.2369362920544025;
	return mph;
}

double Conversions::ConvertDegreesToRadians(double degrees)
{
	double radians = degrees * (M_PI / 180.0);
	return radians;
}

double Conversions::ConvertRadiansToDegrees(double radians)
{
	double degrees = radians * (180.0 / M_PI);
	return degrees;
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

// Return the bearing between two points in degrees from 0 - 360.
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
