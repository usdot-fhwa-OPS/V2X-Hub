
#include "jsoncpp/json/json.h"
#include <memory.h>
#include <chrono>
#include <string>
#include <iostream>
#include <sstream>
#include <tmx/j2735_messages/MapDataMessage.hpp>

namespace CARMAStreetsPlugin
{
    class J2735MapToJsonConverter
    {
    public:
        J2735MapToJsonConverter() = default;
        ~J2735MapToJsonConverter() = default;
        void convertJ2735MAPToMapJSON(const std::shared_ptr<MapData> mapMsgPtr, Json::Value &mapJson) const;
        void convertLanesetToJSON(const IntersectionGeometry *intersection, Json::Value &laneSetJson) const;
        void convertLaneAttributeToJSON(const GenericLane *lane, Json::Value &LaneAttributesJson) const;
        void convertNodeListToJSON(const GenericLane *lane, Json::Value &nodeList) const;
        void convertConnectsToJSON(const GenericLane *lane, Json::Value &nodeListJson) const;
    };
}