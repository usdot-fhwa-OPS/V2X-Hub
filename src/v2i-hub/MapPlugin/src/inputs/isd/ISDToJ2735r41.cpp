/*
 * ISDToJ2735r41.cpp
 *
 *  Created on: May 31, 2017
 *      Author: gmb
 */

#include "ISDToJ2735r41.h"
#include <PluginLog.h>

using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;

namespace MapPlugin {

template <int type>
void add_layer(MapData *map, ISDDataAdaptor &adaptor)
{
	BOOST_THROW_EXCEPTION(TmxException("Unsupported layerType " + to_string(type) + " in JSON map"));
}

/// Fill in the intersection data in the map
template <>
void add_layer<LayerType_intersectionData>(MapData *map, ISDDataAdaptor &adaptor)
{
	// Loop through the intersection geometry nodes
	IntersectionGeometryAdaptor geom = adaptor.get_IntersectionGeometry();
	if (!geom.is_empty())
	{
		IntersectionGeometry* intersection = NULL;
		allocate(intersection);
		intersection->id.id = geom.get_intersectionID();
		PLOG(logDEBUG) << "Intersection revision is " << geom.get_msgCount();
		intersection->revision = geom.get_msgCount();
		intersection->refPoint.Long = geom.get_referenceLon() * 10000000;
		intersection->refPoint.lat = geom.get_referenceLat() * 10000000;

		PLOG(logDEBUG) << "Intersection " << intersection->id.id << " reference point at " << intersection->refPoint.lat << "," << intersection->refPoint.Long;

		addOptionalValue(intersection->refPoint.elevation, geom.get_referenceElevation() * 10, -10.0);
		addOptionalValue(intersection->laneWidth, geom.get_masterLaneWidth(), -1L);
		addOptionalValue(intersection->name, geom.get_descriptiveIntersctionName(), string(""));

		// Include the approach lanes
		for (auto approach : geom.get_lanes())
		{
			string type = approach.get_approachType();
			int id = atoi(approach.get_approachID().c_str());

			if (type != "none")
			{
				std::vector<DrivingLaneAdaptor> driveLanes = (type == "Crosswalk" ? approach.get_crosswalkLanes() : approach.get_drivingLanes());
				for (auto driveLane : driveLanes)
				{
					GenericLane* lane = NULL;
					allocate(lane);

					lane->laneID = atol(driveLane.get_laneID().c_str());
					string laneType = driveLane.get_laneType();
					PLOG(logDEBUG) << "Adding " << laneType << " lane " << lane->laneID << " to intersection " << intersection->id.id;

					bitset<2> ld;
					bitset<16> attrs = driveLane.get_typeAttributes();

					if (type == "Ingress" || type == "ingress")
					{
						addOptionalValue(lane->ingressApproach, id, 0);
						ld[LaneDirection_ingressPath] = true;
						// TODO: Sharedwith
					}
					else if (type == "Egress" || type == "egress")
					{
						addOptionalValue(lane->egressApproach, id, 0);
						ld[LaneDirection_egressPath] = true;
					}

					BIT_STRING_t *bStr = NULL;

					if (laneType == "Vehicle")
					{
						lane->laneAttributes.laneType.present = LaneTypeAttributes_PR_vehicle;
						bStr = &(lane->laneAttributes.laneType.choice.vehicle);
					}
					else if (laneType == "Crosswalk")
					{
						lane->laneAttributes.laneType.present = LaneTypeAttributes_PR_crosswalk;
						bStr = &(lane->laneAttributes.laneType.choice.crosswalk);
					}
					else if (laneType == "Bikelane")
					{
						lane->laneAttributes.laneType.present = LaneTypeAttributes_PR_bikeLane;
						bStr = &(lane->laneAttributes.laneType.choice.bikeLane);
					}
					else if (laneType == "Sidewalk")
					{
						lane->laneAttributes.laneType.present = LaneTypeAttributes_PR_sidewalk;
						bStr = &(lane->laneAttributes.laneType.choice.sidewalk);
					}
					else if (laneType == "Median")
					{
						lane->laneAttributes.laneType.present = LaneTypeAttributes_PR_median;
						bStr = &(lane->laneAttributes.laneType.choice.median);
					}
					else if (laneType == "Striping")
					{
						lane->laneAttributes.laneType.present = LaneTypeAttributes_PR_striping;
						bStr = &(lane->laneAttributes.laneType.choice.striping);
					}
					else if (laneType == "TrackedVehicle")
					{
						lane->laneAttributes.laneType.present = LaneTypeAttributes_PR_trackedVehicle;
						bStr = &(lane->laneAttributes.laneType.choice.trackedVehicle);
					}
					else if (laneType == "Parking")
					{
						lane->laneAttributes.laneType.present = LaneTypeAttributes_PR_parking;
						bStr = &(lane->laneAttributes.laneType.choice.parking);
					}

					if (bStr)
						addValue(*bStr, attrs);

					addValue(lane->laneAttributes.directionalUse, ld);
					addValue(lane->laneAttributes.sharedWith, driveLane.get_sharedWith());

					PLOG(logDEBUG) << "Drive lane is " << driveLane << ", lane maneuvers is " << driveLane.get_laneManeuvers().to_string();
					addOptionalValue(lane->maneuvers, driveLane.get_laneManeuvers(), 0);

					lane->nodeList.present = NO_NODE;

					// Node list
					WGS84Point refPoint(geom.get_referenceLat(), geom.get_referenceLon());

					for (auto laneNode : driveLane.get_laneNodes())
					{
						if (lane->nodeList.present == NO_NODE)
							lane->nodeList.present = DEFINED_NODE;

						Node *node = NULL;
						allocate(node);

						WGS84Point nodePoint(laneNode.get_nodeLat(), laneNode.get_nodeLong());
						WGS84Point refXToNodeY(refPoint.Latitude, nodePoint.Longitude);
						WGS84Point nodeXToRefY(nodePoint.Latitude, refPoint.Longitude);

						int modX = GeoVector::DistanceInMeters(nodePoint, nodeXToRefY) * 100;
						if (nodePoint.Longitude < refPoint.Longitude) modX *= -1;
						int modY = GeoVector::DistanceInMeters(nodePoint, refXToNodeY) * 100;
						if (nodePoint.Latitude < refPoint.Latitude) modY *= -1;
						int largestOffset;
						if (abs(modX) > abs(modY))
							largestOffset = modX;
						else
							largestOffset = modY;

						if ((largestOffset >= -512) && (largestOffset <= 511)) {
							node->delta.present = ENUM_NAME(XY1);
							node->delta.choice.node_XY1.x = modX;
							node->delta.choice.node_XY1.y = modY;
						} else if ((largestOffset >= -1024) && (largestOffset <= 1023)) {
							node->delta.present = ENUM_NAME(XY2);
							node->delta.choice.node_XY2.x = modX;
							node->delta.choice.node_XY2.y = modY;
						} else if ((largestOffset >= -2048) && (largestOffset <= 2047)) {
							node->delta.present = ENUM_NAME(XY3);
							node->delta.choice.node_XY3.x = modX;
							node->delta.choice.node_XY3.y = modY;
						} else if ((largestOffset >= -4096) && (largestOffset <= 4095)) {
							node->delta.present = ENUM_NAME(XY4);
							node->delta.choice.node_XY4.x = modX;
							node->delta.choice.node_XY4.y = modY;
						} else if ((largestOffset >= -8192) && (largestOffset <= 8191)) {
							node->delta.present = ENUM_NAME(XY5);
							node->delta.choice.node_XY5.x = modX;
							node->delta.choice.node_XY5.y = modY;
						} else if ((largestOffset >= -32768) && (largestOffset <= 32767)) {
							node->delta.present = ENUM_NAME(XY6);
							node->delta.choice.node_XY6.x = modX;
							node->delta.choice.node_XY6.y = modY;
						}

						refPoint = nodePoint;

						long dWidth = laneNode.get_laneWidthDelta();
						long dElev = laneNode.get_nodeElev() * 10;
						if (dElev <= LaneNodeAdaptor::nodeElev::default_value() || !(intersection->refPoint.elevation))
							dElev = 0;
						else
							dElev += -1 * (*(intersection->refPoint.elevation));

						if (dWidth || dElev)
						{
							allocate(node->attributes);
							addOptionalValue(node->attributes->dWidth, dWidth, 0L);
							addOptionalValue(node->attributes->dElevation, dElev, 0L);
						}

						ASN_SEQUENCE_ADD(&lane->nodeList.choice.nodes, node);
					}

					// Connections
					lane->connectsTo = NULL;
					for (auto conn : driveLane.get_connections())
					{
						// Only consider lanes that connect from this current one
						if (conn.get_fromLane() != driveLane.get_laneID())
							continue;

						if (!lane->connectsTo)
							allocate(lane->connectsTo);

						Connection *connection;
						allocate(connection);

						long signalId = (conn.get_signal_id() == "" ? -1L : atol(conn.get_signal_id().c_str()));

						addOptionalValue(connection->signalGroup, signalId, -1L);

						connection->connectingLane.lane = atol(conn.get_toLane().c_str());
						addOptionalValue(connection->connectingLane.maneuver, conn.get_maneuvers(), 0);

						ASN_SEQUENCE_ADD(&lane->connectsTo->list, connection);
					}

					ASN_SEQUENCE_ADD(&intersection->laneSet, lane);
				}
			}
		}

		allocate(map->intersections);
		ASN_SEQUENCE_ADD(&map->intersections->list, intersection);
	}
}

ISDToJ2735r41::~ISDToJ2735r41() {
}

MapData *ISDToJ2735r41::to_map() {
	PLOG(logDEBUG) << "Creating MapData from ISD file: " << endl << adaptor;

    MapData *map = NULL;
    allocate(map);

    memset(map, 0, sizeof(MapData));
    map->msgIssueRevision = 0;

    // Layer type from string
    decodeEnumFromString(map->layerType, asn_DEF_LayerType, adaptor.get_layerType(), asn_DEF_LayerType.xml_tag);
    if (!map->layerType)
    	return map;

    PLOG(logDEBUG) << "Decoded layer type " << (map->layerType ? to_string(*(map->layerType)) : "????");

    if (map->layerType && *(map->layerType))
    {
    	switch(*(map->layerType))
    	{
    	case LayerType_intersectionData: add_layer<LayerType_intersectionData>(map, adaptor); break;
    	case LayerType_roadwaySectionData: add_layer<LayerType_roadwaySectionData>(map, adaptor); break;
    	case LayerType_curveData: add_layer<LayerType_curveData>(map, adaptor); break;
    	case LayerType_parkingAreaData: add_layer<LayerType_parkingAreaData>(map, adaptor); break;
    	case LayerType_sharedLaneData: add_layer<LayerType_sharedLaneData>(map, adaptor); break;
    	}
    }

    return map;
}

MapDataMessage ISDToJ2735r41::to_message() {
	return MapDataMessage(to_map());
}

MapDataEncodedMessage ISDToJ2735r41::to_encoded_message() {
	MapDataMessage mapMsg = to_message();
	PLOG(logDEBUG) << mapMsg;

	MapDataEncodedMessage encMsg;
	encMsg.initialize(mapMsg);
	return encMsg;
}

} /* namespace MapPlugin */
