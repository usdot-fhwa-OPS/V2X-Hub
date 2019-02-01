/*
 * WGS84Point.h
 *
 *  Created on: Oct 15, 2014
 *      Author: ivp
 */

#ifndef WGS84POINT_H_
#define WGS84POINT_H_

namespace tmx {
namespace utils {

/// WGS84Point encapsulates coordinates on the earths surface.
typedef struct WGS84Point
{
	WGS84Point() : Latitude(0), Longitude(0), Elevation(0) {}

	WGS84Point(double latitude, double longitude, double elevation = 0.0):
		Latitude(latitude), Longitude(longitude), Elevation(elevation) { }

	double Latitude;
	double Longitude;
	double Elevation;
} WGS84Point;

}} // namespace tmx::utils

#endif /* WGS84POINT_H_ */
