
#include "jsoncpp/json/json.h"
#include <memory>
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
        /**
         * @brief Convert the J2735 MAPData into JSON format.
         * @param mapMsgPtr The input is a constant J2735 message pointer. This prevent any modification to the original MAP message
         * @param mapJson Pass by reference to allow the method to populate this object with MAPData.
         */
        void convertJ2735MAPToMapJSON(const std::shared_ptr<MapData> mapMsgPtr, Json::Value &mapJson) const;

        /**
         * @brief Convert the J2735 IntersectionGeometry into JSON format.
         * @param mapMsgPtr The input is a constant J2735 message pointer. This prevent any modification to the original IntersectionGeometry message
         * @param mapJson Pass by reference to allow the method to populate this object with IntersectionGeometry.
         */
        void convertLanesetToJSON(const IntersectionGeometry *intersection, Json::Value &laneSetJson) const;
        /**
         * @brief Convert the J2735 GenericLane into JSON format.
         * @param mapMsgPtr The input is a constant J2735 message pointer. This prevent any modification to the original GenericLane message
         * @param mapJson Pass by reference to allow the method to populate this object with GenericLane.
         */
        void convertLaneAttributeToJSON(const GenericLane *lane, Json::Value &LaneAttributesJson) const;

        /**
         * @brief Convert the J2735 GenericLane into JSON format.
         * @param mapMsgPtr The input is a constant J2735 message pointer. This prevent any modification to the original GenericLane message
         * @param mapJson Pass by reference to allow the method to populate this object with GenericLane.
         */
        void convertNodeListToJSON(const GenericLane *lane, Json::Value &nodeList) const;

        /**
         * @brief Convert the J2735 GenericLane into JSON format.
         * @param mapMsgPtr The input is a constant J2735 message pointer. This prevent any modification to the original GenericLane message
         * @param mapJson Pass by reference to allow the method to populate this object with GenericLane.
         */
        void convertConnectsToJSON(const GenericLane *lane, Json::Value &nodeListJson) const;
    };
}