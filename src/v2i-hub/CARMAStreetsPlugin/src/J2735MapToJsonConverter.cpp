

#include "J2735MapToJsonConverter.h"

namespace CARMAStreetsPlugin
{

    void J2735MapToJsonConverter::convertJ2735MAPToMapJSON(const std::shared_ptr<MapData> mapMsgPtr, Json::Value &mapJson) const
    {
        // Construct metadata
        Json::Value metadata;
        auto timestamp_utc = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        metadata["timestamp"] = std::to_string(timestamp_utc);
        mapJson["metadata"] = metadata;

        // Construct Map Data
        Json::Value mapDataJson;
        mapDataJson["layer_id"] = std::to_string(*mapMsgPtr->layerID);
        mapDataJson["msg_issue_revision"] = std::to_string(mapMsgPtr->msgIssueRevision);
        if(mapMsgPtr->layerType != nullptr)
        {
            mapDataJson["layer_type"] = std::to_string(*mapMsgPtr->layerType);
        }

        // Construct intersections
        const IntersectionGeometryList *intersections = mapMsgPtr->intersections;
        if(intersections != nullptr)
        {
            Json::Value intersectionsJson;
            // Assume there is only one intersection geometry for each intersection
            for (size_t i = 0; i < intersections->list.count; i++)
            {
                Json::Value intersectionJson;
                if (intersections->list.array != nullptr)
                {
                    auto intersection = intersections->list.array[i];
                    intersectionJson["id"]["id"] = std::to_string(intersection->id.id);
                    intersectionJson["lane_width"] = std::to_string(*intersection->laneWidth);
                    intersectionJson["revision"] = std::to_string(intersection->revision);
                    intersectionJson["ref_point"]["lat"] = std::to_string(intersection->refPoint.lat);
                    intersectionJson["ref_point"]["long"] = std::to_string(intersection->refPoint.Long);
                    if(intersection->refPoint.elevation !=nullptr)
                    {
                        intersectionJson["ref_point"]["elevation"] = std::to_string(*intersection->refPoint.elevation);
                    }

                    // Convert Laneset
                    Json::Value laneSetJson;
                    convertLanesetToJSON(intersection, laneSetJson);
                    intersectionJson["lane_set"] = laneSetJson;
                }
                intersectionsJson["intersection_geometry"] = intersectionJson;
            }
            mapDataJson["intersections"] = intersectionsJson;
        }
        mapJson["map_data"] = mapDataJson;
    }

    void J2735MapToJsonConverter::convertLanesetToJSON(const IntersectionGeometry *intersection, Json::Value &laneSetJson) const
    {
        // Construct laneset
        const auto laneSet = intersection->laneSet;
        if (laneSet.list.array != nullptr)
        {
            for (size_t i = 0; i < laneSet.list.count; i++)
            {
                std::stringstream ss;
                Json::Value laneJson;
                auto lane = laneSet.list.array[i];
                laneJson["lane_id"] = std::to_string(lane->laneID);
                if (lane->ingressApproach != nullptr)
                {
                    ss.str("");
                    ss << *lane->ingressApproach;
                    laneJson["ingress_approach"] = ss.str();
                }
                if (lane->egressApproach != nullptr)
                {
                    ss.str("");
                    ss << *lane->egressApproach;
                    laneJson["egressApproach"] = ss.str();
                }

                // Construct LaneAttributes
                Json::Value LaneAttributesJson;
                convertLaneAttributeToJSON(lane, LaneAttributesJson);
                laneJson["lane_attributes"] = LaneAttributesJson;

                // Construct nodelist
                Json::Value nodeList;
                convertNodeListToJSON(lane, nodeList);
                laneJson["node_list"]["nodes"] = nodeList;

                // Construct connects
                Json::Value connectsJson;
                if (lane->connectsTo != nullptr)
                {
                    convertConnectsToJSON(lane, connectsJson);
                    laneJson["connects_to"] = connectsJson;
                }

                laneSetJson.append(laneJson);
            }
        }
    }

    void J2735MapToJsonConverter::convertLaneAttributeToJSON(const GenericLane *lane, Json::Value &LaneAttributesJson) const
    {
        std::stringstream ss;
        ss.str("");
        auto bit_unused = lane->laneAttributes.directionalUse.bits_unused;
        ss << lane->laneAttributes.directionalUse.buf;
        if( lane->laneAttributes.directionalUse.size != 0 )
        {
            auto binary = lane->laneAttributes.directionalUse.buf[0] >> bit_unused;
            std::string binary_str = std::to_string(static_cast<unsigned>(binary / 2)) + std::to_string(static_cast<unsigned>(binary % 2));
            LaneAttributesJson["directional_use"] = binary_str;
        }
        ss.str("");
        ss << lane->laneAttributes.sharedWith.buf;
        LaneAttributesJson["shared_with"] = ss.str();
        if (lane->laneAttributes.laneType.present == LaneTypeAttributes_PR_vehicle)
        {
            ss.str("");
            ss << lane->laneAttributes.laneType.choice.vehicle.buf;
            LaneAttributesJson["lane_type"]["vehicle"] = ss.str();
        }
        else if (lane->laneAttributes.laneType.present == LaneTypeAttributes_PR_crosswalk)
        {
            ss.str("");
            ss << lane->laneAttributes.laneType.choice.crosswalk.buf;
            LaneAttributesJson["lane_type"]["crosswalk"] = ss.str();
        }
        else if (lane->laneAttributes.laneType.present == LaneTypeAttributes_PR_bikeLane)
        {
            ss.str("");
            ss << lane->laneAttributes.laneType.choice.bikeLane.buf;
            LaneAttributesJson["lane_type"]["bike_lane"] = ss.str();
        }
        else if (lane->laneAttributes.laneType.present == LaneTypeAttributes_PR_median)
        {
            ss.str("");
            ss << lane->laneAttributes.laneType.choice.median.buf;
            LaneAttributesJson["lane_type"]["median"] = ss.str();
        }
        else if (lane->laneAttributes.laneType.present == LaneTypeAttributes_PR_parking)
        {
            ss.str("");
            ss << lane->laneAttributes.laneType.choice.parking.buf;
            LaneAttributesJson["lane_type"]["parking"] = ss.str();
        }
        else if (lane->laneAttributes.laneType.present == LaneTypeAttributes_PR_sidewalk)
        {
            ss.str("");
            ss << lane->laneAttributes.laneType.choice.sidewalk.buf;
            LaneAttributesJson["lane_type"]["sidewalk"] = ss.str();
        }
        else if (lane->laneAttributes.laneType.present == LaneTypeAttributes_PR_striping)
        {
            ss.str("");
            ss << lane->laneAttributes.laneType.choice.striping.buf;
            LaneAttributesJson["lane_type"]["striping"] = ss.str();
        }
        else if (lane->laneAttributes.laneType.present == LaneTypeAttributes_PR_trackedVehicle)
        {
            ss.str("");
            ss << lane->laneAttributes.laneType.choice.trackedVehicle.buf;
            LaneAttributesJson["lane_type"]["tracked_vehicle"] = ss.str();
        }
        else
        {
            ss.str("");
            LaneAttributesJson["lane_type"]["nothing"] = ss.str();
        }
    }

    void J2735MapToJsonConverter::convertNodeListToJSON(const GenericLane *lane, Json::Value &nodeListJson) const
    {
        std::stringstream ss;
        auto nodes = lane->nodeList.choice.nodes;
        if (NodeListXY_PR_nodes == lane->nodeList.present)
        {
            for (size_t i = 0; i < nodes.list.count; i++)
            {
                Json::Value nodeJson;
                auto node = nodes.list.array[i];
                if (node->delta.present == NodeOffsetPointXY_PR::NodeOffsetPointXY_PR_node_XY1)
                {
                    ss.str("");
                    ss << node->delta.choice.node_XY1.x;
                    nodeJson["delta"]["node-xy"]["x"] = ss.str();

                    ss.str("");
                    ss << node->delta.choice.node_XY1.y;
                    nodeJson["delta"]["node-xy"]["y"] = ss.str();
                    nodeListJson.append(nodeJson);
                }
                else if (node->delta.present == NodeOffsetPointXY_PR::NodeOffsetPointXY_PR_node_XY2)
                {
                    ss.str("");
                    ss << node->delta.choice.node_XY2.x;
                    nodeJson["delta"]["node-xy"]["x"] = ss.str();

                    ss.str("");
                    ss << node->delta.choice.node_XY2.y;
                    nodeJson["delta"]["node-xy"]["y"] = ss.str();
                    nodeListJson.append(nodeJson);
                }
                else if (node->delta.present == NodeOffsetPointXY_PR::NodeOffsetPointXY_PR_node_XY3)
                {
                    ss.str("");
                    ss << node->delta.choice.node_XY3.x;
                    nodeJson["delta"]["node-xy"]["x"] = ss.str();

                    ss.str("");
                    ss << node->delta.choice.node_XY3.y;
                    nodeJson["delta"]["node-xy"]["y"] = ss.str();
                    nodeListJson.append(nodeJson);
                }
                else if (node->delta.present == NodeOffsetPointXY_PR::NodeOffsetPointXY_PR_node_XY4)
                {
                    ss.str("");
                    ss << node->delta.choice.node_XY4.x;
                    nodeJson["delta"]["node-xy"]["x"] = ss.str();

                    ss.str("");
                    ss << node->delta.choice.node_XY4.y;
                    nodeJson["delta"]["node-xy"]["y"] = ss.str();
                    nodeListJson.append(nodeJson);
                }
                else if (node->delta.present == NodeOffsetPointXY_PR::NodeOffsetPointXY_PR_node_XY5)
                {
                    ss.str("");
                    ss << node->delta.choice.node_XY5.x;
                    nodeJson["delta"]["node-xy"]["x"] = ss.str();

                    ss.str("");
                    ss << node->delta.choice.node_XY5.y;
                    nodeJson["delta"]["node-xy"]["y"] = ss.str();
                    nodeListJson.append(nodeJson);
                }
                else if (node->delta.present == NodeOffsetPointXY_PR::NodeOffsetPointXY_PR_node_XY6)
                {
                    ss.str("");
                    ss << node->delta.choice.node_XY6.x;
                    nodeJson["delta"]["node-xy"]["x"] = ss.str();

                    ss.str("");
                    ss << node->delta.choice.node_XY6.y;
                    nodeJson["delta"]["node-xy"]["y"] = ss.str();
                    nodeListJson.append(nodeJson);
                }
            }
        }
    }

    void J2735MapToJsonConverter::convertConnectsToJSON(const GenericLane *lane, Json::Value &connectsJson) const
    {
        std::stringstream ss;
        if (lane->connectsTo != nullptr)
        {
            for (size_t i = 0; i < lane->connectsTo->list.count; i++)
            {
                Json::Value connJson;
                ss.str("");
                auto connect = lane->connectsTo->list.array[i];
                ss << connect->connectingLane.lane;
                connJson["connecting_lane"]["lane"] = ss.str();

                ss.str("");
                connect = lane->connectsTo->list.array[i];
                ss << *connect->signalGroup;
                connJson["signal_group"] = ss.str();
                connectsJson.append(connJson);
            }
        }
    }
}