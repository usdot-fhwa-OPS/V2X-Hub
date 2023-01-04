/*
 * VehicleLocate.cpp
 *
 *  Created on: Oct 14, 2014
 *      Author: ivp
 */

#include <iostream>
#include <math.h>

#include "Conversions.h"
#include "VehicleLocate.h"

//#define DSRC_POLAR_RADIUS_CM 635675231.4247833
#define DSRC_EQUATORIAL_RADIUS_CM 637813700
#define PI 3.141592653589793238462643383
#define PI_2 1.57079632679

WGS84Point* VehicleLocate::GetPointArray(NodeSetXY *nodeSet, int nodesLength, Position3D *nodesAnchor)
{
	WGS84Point *points = (WGS84Point *)calloc(nodesLength, sizeof(WGS84Point));

	double baseLatitude = nodesAnchor->lat / 10000000.0;
	double baseLongitude = nodesAnchor->Long / 10000000.0;

	double totalXOffset = 0.0;
	double totalYOffset = 0.0;
	double xOffset;
	double yOffset;

	std::cout << "baseLatitude: " << baseLatitude <<" , baseLongitude"<<baseLongitude<<std::endl;

	for (int i = 0; i < nodesLength; i++)
	{
		xOffset = nodeSet->list.array[i]->delta.choice.node_XY6.x;
		yOffset = nodeSet->list.array[i]->delta.choice.node_XY6.y;

		std::cout << "xOffset " << xOffset << ", yOffset " << yOffset << std::endl;
		totalXOffset += xOffset;
		totalYOffset += yOffset;

		points[i].Latitude = baseLatitude + (totalYOffset / DSRC_EQUATORIAL_RADIUS_CM) *(180/PI);
		points[i].Longitude = baseLongitude + (totalXOffset / DSRC_EQUATORIAL_RADIUS_CM) *(180/PI)/cos(baseLatitude * PI/180);

		std::cout << "points[i].Latitude " << points[i].Latitude << ", points[i].Longitude " << points[i].Longitude << std::endl;
	}

	return points;
}

int VehicleLocate::FindRegion(TravelerInformation *tim, WGS84Point point, uint16_t heading)
{
	for (int i = 0; i < tim->dataFrames.list.count; i++)
	{
		TiDataFrame *frame = tim->dataFrames.list.array[i];

		int regionNumber = FindRegion(frame, point, heading);
		if (regionNumber != -1)
			return regionNumber;
	}
	return -1;
}

int VehicleLocate::FindRegion(TiDataFrame *frame, WGS84Point point, uint16_t heading)
{
	for (int i = 0; i < frame->regions.list.count; i++)
	{
		GeographicalPath *geoPath = frame->regions.list.array[i];

		Position3D *nodesAnchor = geoPath->anchor;
		LaneWidth_t laneWidth = *(geoPath->laneWidth);

		NodeSetXY *nodes = &geoPath->description->choice.path.offset.choice.xy.choice.nodes;
		int nodesLength = geoPath->description->choice.path.offset.choice.xy.choice.nodes.list.count;

		WGS84Point* points = GetPointArray(nodes, nodesLength, nodesAnchor);
		bool inList = IsInPointList(points, nodesLength, point, heading, laneWidth/100.0);
		free(points);
		if (inList) {
			return i + 1;
		}
	}

	return -1;
}

bool VehicleLocate::IsInPointList(WGS84Point* points, int pointsLength, WGS84Point point, uint16_t heading, double laneWidth)
{
	
	for (int i = 1; i < pointsLength; i++)
	{
		double pointsHeading = Conversions::GetBearingDegrees(points[i], points[i-1]);

		std::cout << "CSW IsInPointList - Vehicle Heading: " << heading << ", Points Heading: " << pointsHeading << std::endl;

		// Continue if the two headings are not within +-90 degrees of each other.
		double headingDiff = abs(heading - pointsHeading);
		if (headingDiff > 90 && headingDiff < 270)
			continue;

		double laneDist_m = Conversions::DistanceMeters(points[i], points[i-1]);
		double dist1 = Conversions::DistanceMeters(points[i], point);
		double dist2 = Conversions::DistanceMeters(points[i-1], point);

		std::cout<<"CSW IsInPointList - point: " << point.Latitude <<", " <<point.Longitude<<std::endl;

		std::cout << "CSW IsInPointList - laneDist_m: " << laneDist_m << ", dist1: " << dist1 << ", dist2: " << dist2 << std::endl;

		if ((laneDist_m > dist1) && (laneDist_m > dist2))
		{
			double s = (.5) * (laneDist_m + dist1 + dist2);
			double area = sqrt((s * (s - dist1) * (s - dist2) * (s - laneDist_m)));

			double dist_perp = 2 * (area / laneDist_m);

			std::cout << "CSW IsInPointList - laneWidth: " << laneWidth << ", dist_perp: " << dist_perp << std::endl;

			if (laneWidth / 2 > dist_perp)
			{
				return true;
			}
		}
	}

	return false;
}

void VehicleLocate::GetInt16(unsigned char *buf, int16_t *value)
{
	*value = (int16_t)((buf[0] << 8) + buf[1]);
}

void VehicleLocate::GetUInt16(unsigned char *buf, uint16_t *value)
{
	*value = (uint16_t)((buf[0] << 8) + buf[1]);
}

void VehicleLocate::GetInt32(unsigned char *buf, int32_t *value)
{
	*value = (int32_t)((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
}

void VehicleLocate::GetUInt32(unsigned char *buf, uint32_t *value)
{
	*value = (uint32_t)((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
}

// Process a BSM.
// Extract and return in the parameters:
// - regionNumber: The number of the TIM region the vehicle is within or 0 if not in a region.
// - speed_mph: The current speed of the vehicle.
// - vehicleId: The ID of the vehicle.
// Returns true on success; false if the BSM is invalid or other error occurs.
bool VehicleLocate::ProcessBsmMessage(BasicSafetyMessage_t &bsm, TravelerInformation *tim, int* regionNumber, float* speed_mph, int32_t *vehicleId)
{
	bool isSuccess = false;
	//asn_fprint(stdout, &asn_DEF_BasicSafetyMessage, bsm);

	int32_t latitude = bsm.coreData.lat;
	int32_t longitude = bsm.coreData.Long;

std::cout<<"ProgessBsmMessage - lat: "<<latitude<<" ,"<<" lon: "<<longitude<<std::endl;

	uint16_t rawSpeed = bsm.coreData.speed;
	uint16_t rawHeading = bsm.coreData.heading;
	GetInt32((unsigned char *)bsm.coreData.id.buf, vehicleId);

	WGS84Point vehiclePoint;
	vehiclePoint.Latitude = (double)latitude/10000000;
	vehiclePoint.Longitude = (double)longitude/10000000;

	// Heading units are 0.0125 degrees.
	float heading = rawHeading / 80.0;

	// The speed is contained in bits 0-12.  Units are 0.02 meters/sec.
	// A value of 8191 is used when the speed is not known.
	if (rawSpeed != 8191)
	{
		// Convert from .02 meters/sec to mph.
		*speed_mph = rawSpeed / 50 * 2.2369362920544;

		//std::cout << "Vehicle Lat/Long/Heading/Speed: " << vehiclePoint.Latitude << ", " << vehiclePoint.Longitude << ", " << heading << ", " << speed << std::endl;

		*regionNumber = FindRegion(tim, vehiclePoint, heading);

		if (*regionNumber < 0)
			*regionNumber = 0;

		//std::cout << "Vehicle Speed: " << *speed_mph << ", Region: " << *regionNumber << std::endl;

		isSuccess = true;
	}

	return isSuccess;
}

