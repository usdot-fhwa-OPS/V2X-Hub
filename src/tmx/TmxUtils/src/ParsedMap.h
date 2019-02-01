/*
 * ParsedMap.h
 *
 *  Created on: May 5, 2016
 *      Author: ivp
 */

#ifndef SRC_PARSEDMAP_H_
#define SRC_PARSEDMAP_H_

#include <list>
#include <sstream>
#include "WGS84Point.h"

namespace tmx {
namespace utils {

/**Lat/Long of the point computed from:
 *
 * RawMAP data:The geometry of a lane is described by a list of nodes (always at least two) each with a
 * Northerly and Easterly offset (positive values) or Southerly and Westerly offsets (negative values).
 *  The offsets for the first node are relative to the intersections reference point that is given
 *  as a lat/long position, the offsets for all remaining nodes, after the first one, are relative
 *   to the previous node.  You should typically set you offset resolution to decimeter.
 *
 */
class LaneNode {
public:
	LaneNode() {}
	LaneNode(double latitude, double longitude)
	{
		Point.Latitude = latitude;
		Point.Longitude = longitude;
	}

	WGS84Point Point;
};

enum  LaneType {
	Other = 0,
	Vehicle = 1,
	Computed = 2,
	///crosswalk
	Pedestrian = 3,
	///computed egress
	Egress = 4,
	///curb
	Sidewalk = 5
};

enum DirectionalUse {
	NotApplicable = 0,
	///ingress
	Ingress_Vehicle_Computed = 1,
	///direction irrelevant for pedestrian and sidewalk
	Ingress_Pedestrian = 2,
	///egress
	Egress_Computed = 3
};

class MapLane {
public:
	int LaneNumber;
	double LaneWidthMeters;
	///LaneDirection_ingressPath	= 0,
	///LaneDirection_egressPath	= 1
	bool LaneDirectionEgress;
	std::list<LaneNode> Nodes;
	//Note: Directional Use is 10 and Node List has nodes then Lane Type is Ingress Vehicle
	//Note: Directional Use is 10 and Node List has the computed tag than Lane Type is Ingress Computed
	//Note: Directional Use is 11 and then the Lane Type is Ingress Pedestrian
	//Note: Directional Use is 01 and then the Lane Type is Egress Computed.
	LaneType Type;
	//Note: 10 is an Ingress lane (can be two lane types see above), 11 is a Pedestrian lane and 01 is and Egress lane
	DirectionalUse Direction;
	int ReferenceLaneId;

	/*
	 * From the MAP message indicating the group that needs to be queried in the SPAT message to find
	 * the state of the traffic signal for this lane.
	 * Under laneSet->connectsTo->signalGroup
	 */
	int SignalGroupId;
	WGS84Point LaneNodeOffset;

	/// string representation for debug output.
	std::string ToString()
	{
		std::ostringstream ss;
		ss.setf(std::ios::boolalpha);
		ss << "Number: " << LaneNumber << ", Width: " << LaneWidthMeters << " m, Type: " << Type << ", ";
		ss << "Direction Egress: " << LaneDirectionEgress << ", Directional Use: " << Direction << ", ";
		ss << "Signal Group ID: " << SignalGroupId << ", Nodes: ";
		for (auto node: Nodes)
		{
			ss << "(" << node.Point.Latitude << "," << node.Point.Longitude << ") ";
		}
		return ss.str();
	}
};
/*ParsedMap is a 'smarter' version of the raw MAP data we get from the DSRC data.
 * ParsedMap translates all offsets, computed lanes, and raw points into true lat/longs.
 * It also populates the fields that are needed for calculations.
 *
 * Raw MAP data:All geometry points are Cartesian offsets from an intersection reference point that is
given in (Latitude, Longitude, and Altitude) coordinates in the WGS 84 system. This
means that all the points that are used to describe the geometry are described as distance
in decimeters from the intersection reference point (x [decimeters], y [decimeters],
z [decimeters]);
 *
 */
class ParsedMap {
public:
	WGS84Point ReferencePoint;
	std::list<MapLane> Lanes;

	///For creating a bounding box for entire MAP.
	double MaxLat=0;
	double MinLat=0;
	double MaxLong=0;
	double MinLong=0;
};

}} // namespace tmx::utils

#endif /* SRC_PARSEDMAP_H_ */
