/*
 * MapSupport.cpp
 *
 *  Created on: Apr 21, 2016
 *      Author: ivp
 */

#include "MapSupport.h"
#include <cmath>

#include "PluginLog.h"
#include "GeoVector.h"

//#include "DsrcUtils.h"
//#include "Queue.h"

//#include <stdio.h>
//#include <string.h>
//#include <pthread.h>
//#include <endian.h>

//#include <TravelerInformation.h>
//#include <AlaCarte.h>
//#include <wave.h>

using namespace std;

namespace tmx {
namespace utils {

MapSupport::MapSupport(): _irExtent(0) {
	// TODO Auto-generated constructor stub

}

MapSupport::~MapSupport() {
	// TODO Auto-generated destructor stub
}

/**
 * Reference point at center of intersection. Determines if vehicle is approaching
 * or departing the intersection
 */
void IsApproachingRefPoint(WGS84Point previousPoint, WGS84Point currentPoint) {

}

bool MapSupport::IsVehicleLane(int laneId, ParsedMap &map) {
	bool isVehicleLane = false;

	for (std::list<MapLane>::iterator i = map.Lanes.begin();
			i != map.Lanes.end(); i++) {
		//Check all three types of lanes that stand for vehicle lanes.
		if (laneId == i->LaneNumber
				&& (i->Type == Vehicle || i->Type == Computed
						|| i->Type == Egress)) {
			isVehicleLane = true;
			break;
		}
	}

	return isVehicleLane;
}

/*
 * MAP file does not always specify offsets in decimeters. Pull this factor out of xml and use it.
 */
void ExtractOffsetConversionFactor() {
	//todo
}

/**
 * Returns -2 if not on the map. -1 if not in a lane. 0 if in the intersection itself. else, lane id matched.
 */
bool MapSupport::IsPointInLane(WGS84Point point, int laneId,
		ParsedMap &map) {

	//Iterate over all lanes.
	list<MapLane>::iterator i;
	//Iterate over all vehicle lanes in the region.
	for (i = map.Lanes.begin(); i != map.Lanes.end(); ++i) {
		if (i->LaneNumber == laneId) {
			MapMatchResult result = PointIsInLane(*i, point);
			if (result.IsInLane) {
				return true;;
			}
		}
	}

	return false;
}

/**
 * Returns -2 if not on the map. -1 if not in a lane. 0 if in the intersection itself. else, lane id matched.
 */
MapMatchResult MapSupport::FindVehicleLaneForPoint(WGS84Point point,
		ParsedMap &map) {
	MapMatchResult r;
	r.PerpDistanceMeters = 0;
	r.StopDistanceMeters = 0;
	r.IsInLane = false;
	r.LaneSegment = 0;
	r.IsEgress = false;

	//First see if the point is inside the loose bounds of the MAP at all.
	if(!IsPointOnMapUsa(point,map))
	{
		r.LaneNumber = -2; //Not on the map.
		return r;
	}

	//Iterate over all lanes.
	list<MapLane>::iterator i;
	//Iterate over all vehicle lanes in the region.
	for (i = map.Lanes.begin(); i != map.Lanes.end(); ++i) {
		if (IsVehicleLane(i->LaneNumber, map)) {
			MapMatchResult result = PointIsInLane(*i, point);
			if (result.IsInLane) {
				//PLOG(logDEBUG) << "MapMatchResult: " << result.IsInLane << ", " << result.LaneNumber << ", " << result.LaneSegment << ", " << result.PerpDistanceMeters << ", " << result.StopDistanceMeters << ", " << result.IsEgress;
				return result;
			}
		}
	}
	//We have not matched to a lane. See if we are actually within the intersection
	if (IsInCenterOfIntersection(point, map)) {

		r.LaneNumber = 0; //return 0 to represent being within the intersection itself.

		return r;
	}
	r.LaneNumber = -1; //return -1 to represent not found.
	return r;
}
//Loose check for being near/on the map
bool MapSupport::IsPointOnMapUsa(WGS84Point point,ParsedMap &map) {

	if(point.Latitude > map.MaxLat || point.Latitude < map.MinLat
			|| point.Longitude > map.MaxLong || point.Longitude < map.MinLong){
		//point exceeded bounds.
		return false;
}
return true;
}

int MapSupport::GetSignalGroupForVehicleLane(int laneId, ParsedMap &map) {
	int signalGroup = -1;

	for (std::list<MapLane>::iterator i = map.Lanes.begin();
			i != map.Lanes.end(); i++) {
		if (laneId == i->LaneNumber
				&& (i->Type == Vehicle || i->Type == Computed)) {
			signalGroup = i->SignalGroupId;
			break;
		}
	}

	return signalGroup;
}

/*
 * When actually IN the intersection, the vehicle may not be in a "lane" but certainly
 * is still in important territory to monitor.
 */
bool MapSupport::IsInCenterOfIntersection(WGS84Point point, ParsedMap &map) {
	//For draft 2, lets check to see if we are in a lane.  If we have a lane number, 
	//then return false since we are not in the center of the intersection.
	//Find the radius of the circle defining the center of the intersection, then
	//check to see if we are within that radius.

	double radius = 0;
	
	for(MapLane mapLane : map.Lanes)
	{
		LaneNode firstNode = mapLane.Nodes.front();

		double tempDist = Conversions::DistanceMeters(map.ReferencePoint, firstNode.Point);

		if(tempDist > radius)
			radius = tempDist;
	}

	double dist = Conversions::DistanceMeters(map.ReferencePoint, point);
	if (dist < radius * (1 + _irExtent)) {
		return true;
	}

	return false;
}

MapMatchResult MapSupport::PointIsInLane(MapLane &lane, WGS84Point point)
{
	bool isFirstPoint = true;
	int laneSegment = 0;
	WGS84Point p1;
	WGS84Point p2;
	double crossTrackDistance;
	MapMatchResult res;
	double stopBarDistance = 0.0;
	for (auto it=lane.Nodes.begin(); it != lane.Nodes.end(); ++it)
	{
		p1 = p2;
		p2 = it->Point;
		if (!isFirstPoint)
		{
			laneSegment++;
			//get cross track distance
			crossTrackDistance = GeoVector::CrossTrackDistanceInMeters(point, p1, p2);
			//check if point is within width of lane
			if (fabs(crossTrackDistance) <= lane.LaneWidthMeters / 2.0)
			{
//				//check is point is between segment ends, angle method
//				double angle = GeoVector::AngleBetweenPathsInDegrees(p1, p2, p1, point);
//				if (angle <= 90.0 && angle >= -90.0)
//				{
//					angle = GeoVector::AngleBetweenPathsInDegrees(p2, p1, p2, point);
//					if (angle <= 90.0 && angle >= -90.0)
//					{
//						//point is in lane
//						res.IsInLane = true;
//						res.PerpDistanceMeters = fabs(crossTrackDistance);
//						res.LaneNumber = lane.LaneNumber;
//						res.IsEgress = lane.Direction == Egress_Computed ? true : false;
//						res.LaneSegment = laneSegment;
//						res.StopDistanceMeters = stopBarDistance + GeoVector::DistanceInMeters(p1, point);
//						return res;
//					}
//				}
				//check is point is between segment ends
				if (GeoVector::IsBetween(point, p1, p2))
				{
					//point is in lane
					res.IsInLane = true;
					res.PerpDistanceMeters = fabs(crossTrackDistance);
					res.LaneNumber = lane.LaneNumber;
					res.IsEgress = lane.Direction == Egress_Computed ? true : false;
					res.LaneSegment = laneSegment;
					res.StopDistanceMeters = stopBarDistance + GeoVector::DistanceInMeters(p1, GeoVector::NearestPointOnSegment(point, p1, p2));
					return res;
				}
				//check if point is in dead space between segments of a curved lane
				if (laneSegment > 1 && GeoVector::DistanceInMeters(p1, point) <= lane.LaneWidthMeters / 2.0)
				{
					//point is in dead space between this lane segment and previous lane segment
					res.IsInLane = true;
					res.PerpDistanceMeters = fabs(crossTrackDistance);
					res.LaneNumber = lane.LaneNumber;
					res.IsEgress = lane.Direction == Egress_Computed ? true : false;
					res.LaneSegment = laneSegment;
					res.StopDistanceMeters = stopBarDistance;
					return res;
				}
			}
			//if point not in this segment add segment length to stopBarDistance
			stopBarDistance += GeoVector::DistanceInMeters(p1, p2);
		}
		else
			isFirstPoint = false;
	}
	return res;
}

//MapMatchResult MapSupport::PointIsInLane(MapLane &lane, WGS84Point point) {
////Get all the lane Nodes
////--	MapLaneNode[] laneNodes = lane.getLaneNodes().toArray(new MapLaneNode[0]);
//
//	//cout << "Lane: " << lane.ToString() << endl;
//
////Walk through all the lane nodes (actually this walks through the lane segments)
//	//--for (int i = 1; i < laneNodes.length; i++) {
//	list<LaneNode>::iterator n;
//	//Iterate over all vehicle lanes in the region.
//	n = lane.Nodes.begin();
//
//	WGS84Point firstPointInLane = n->Point;//store this one off, need it for stopLane distance.
//	WGS84Point nodeStart = n->Point;
//	int laneSegment = 0;
//	//Compares node point to previous so start one in.
//	for (n++; n != lane.Nodes.end(); ++n) {
//		laneSegment++;
//		WGS84Point nodeEnd = n->Point;
//		// We need to build our triangle.  Get the distance of each lane segment, and the distance from
//		// me to each end of the lane segment
//		double laneDist_m = Conversions::DistanceMeters(nodeEnd, nodeStart);
//		double dist1 = Conversions::DistanceMeters(nodeEnd, point);
//		double dist2 = Conversions::DistanceMeters(nodeStart, point);
//		double halfLaneWidth = lane.LaneWidthMeters / 2;
//
//		//We want to narrow down the further math we need to do to only points in the vicinity of the
//		//line segment.
//		//We could check if each node-to-point distance is less than the length of the segment - this
//		//would mean that we were "inside" the lane segment (think x dimension).
//		//However, this check fails as the point moves to the left or right to the extreme that the
//		//triangle becomes a right triangle.  Because for a right triangle, where dist1 is the halfLanewidth
//		//then clearly dist2 is the hypotenuse of that right triangle and will be more than the segment length.
//
//		//We want to allow this point in, but keep the math simple since this is just to narrow down the points,
//		//so if we add the halfLaneWidth to teh allowance, this allows MORE than enough to cover how much longer
//		//that hypotenuse would be if it were the extreme case. It still rejects a lot of points though.
//		if ((laneDist_m + halfLaneWidth > dist1)
//				&& (laneDist_m + halfLaneWidth > dist2)) {
//			//However this does let in obtuse triangles where the point really lines up better with another
//			//segment (e.g. the point is 'past' the segment in x.  So lets add another check to ensure the
//			//triangle is not obtuse.
//			//In triangle ABC, if c is the longest side of the triangle, then
//			//Acute: c^2 < a^2 + b^2
//			//Right: c^2 = a^2 + b^2
//			//Obtuse: c^2 > a^2 + b^2
//			bool isObtuse = false;
//			double longest = dist1 > dist2 ? dist1 : dist2;
//			double shortest = dist1 > dist2 ? dist2 : dist1;
//
//			double ab = pow(laneDist_m, 2) + pow(shortest, 2);
//			double c = pow(longest, 2);
//			if (c > ab)
//				isObtuse = true;
//
//			if (!isObtuse) {
//
//				/**
//				 * A method for calculating the area of a triangle when you know the lengths of all three sides.
//
//				 Let a,b,c be the lengths of the sides of a triangle. The area is given by:
//				 Area	=	 √	 p	 (	p	−	a	) 	(	p	−	b	)	 (	p	−	c	)
//				 where p is half the perimeter, or
//				 a	+	b	+	c
//				 _________________
//				 2
//
//
//				 */
//				double s = (.5) * (laneDist_m + dist1 + dist2);
//				double area = sqrt(
//						(s * (s - dist1) * (s - dist2) * (s - laneDist_m)));
//
//				// using geometry, find the perpendicular distance from me to the lane
//				/**
//				 * Usually called "half of base times height", the area of a triangle is given by the formula below.
//				 Area	=	b*a
//				 ----
//				 2
//
//				 where
//				 b  is the length of the base
//				 a  is the length of the corresponding altitude
//				 */
//
//				double dist_perp = 2 * (area / laneDist_m);
//
//				// We need to be 1/2 of the lane width distance from the center line to be in the lane
//				if (dist_perp <= halfLaneWidth) {
//					// this built a return type with the lane number that I am in, with the distance from the lane
//					// center line
////				MapMessage.MapLaneCheckReturnType
////				returnType = new MapLaneCheckReturnType();
////				returnType.setDistance_meters(dist_perp);
////				returnType.setIsInLane(true);
////				returnType.setLaneNumber(lane.getLaneNumber());
//
//					MapMatchResult res;
//					res.IsInLane = true;
//					res.PerpDistanceMeters = dist_perp;
//					res.LaneNumber = lane.LaneNumber;
//					res.IsEgress = lane.Direction == Egress_Computed ? true : false;
//					res.LaneSegment = laneSegment;			//save laneSegment
//					//First point in lane was saved, calculate the distance from our point to first point in lane (which is the stop bar of the intersection).
//					res.StopDistanceMeters = Conversions::DistanceMeters(firstPointInLane, point);
//					return res;
//				}
//			}
//		}
//
//		//Set up for next iteration
//		nodeStart = n->Point;
//	}
//
//	//This returns not in lane
////	MapMessage.MapLaneCheckReturnType
////	returnType = new MapLaneCheckReturnType();
////	returnType.setDistance_meters(-1);
////	returnType.setIsInLane(false);
////	returnType.setLaneNumber(lane.getLaneNumber());
//
//	MapMatchResult res;
//	res.LaneNumber = lane.LaneNumber;
//	return res;
//}

void MapSupport::SetExtendedIntersectionPercentage(double percent)
{
	_irExtent = percent;
}

}
} // namespace tmx::utils
