#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <jsoncpp/json/json.h>
#include "RSUTopicsMessage.h"

namespace TelematicBridge
{
    /**
     * @brief TRU Topics Message containing unit ID, RSU topics list, and timestamp
     */
    class TRUTopicsMessage
    {
    private:
        std::string _unitId;
        std::vector<RSUTopicsMessage> _rsuTopics;
        int64_t _timestamp;

    public:
        TRUTopicsMessage() : _unitId(""), _timestamp(0) {}

        TRUTopicsMessage(const std::string& unitId, const std::vector<RSUTopicsMessage>& rsuTopics, int64_t timestamp)
            : _unitId(unitId), _rsuTopics(rsuTopics), _timestamp(timestamp) {}

        /**
         * @brief Get the unit ID
         * @return const std::string& Reference to unit ID
         */
        const std::string& getUnitId() const { return _unitId; }

        /**
         * @brief Set the unit ID
         * @param unitId Unit identifier string
         */
        void setUnitId(const std::string& unitId) { _unitId = unitId; }

        /**
         * @brief Get the RSU topics list
         * @return const std::vector<RSUTopicsMessage>& Reference to RSU topics vector
         */
        const std::vector<RSUTopicsMessage>& getRsuTopics() const { return _rsuTopics; }

        /**
         * @brief Set the RSU topics list
         * @param rsuTopics Vector of RSUTopicsMessage objects
         */
        void setRsuTopics(const std::vector<RSUTopicsMessage>& rsuTopics) { _rsuTopics = rsuTopics; }

        /**
         * @brief Add an RSU topics message to the list
         * @param rsuTopic RSUTopicsMessage to add
         */
        void addRsuTopic(const RSUTopicsMessage& rsuTopic) { _rsuTopics.push_back(rsuTopic); }

        /**
         * @brief Get the timestamp
         * @return int64_t Timestamp in milliseconds
         */
        int64_t getTimestamp() const { return _timestamp; }

        /**
         * @brief Set the timestamp
         * @param timestamp Timestamp in milliseconds
         */
        void setTimestamp(int64_t timestamp) { _timestamp = timestamp; }

        /**
         * @brief Set the timestamp to current time in milliseconds
         */
        void setCurrentTimestamp()
        {
            auto now = std::chrono::system_clock::now();
            _timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();
        }

        /**
         * @brief Convert TRUTopicsMessage to JSON object
         * @return Json::Value JSON representation with structure:
         *         {
         *           "unitId": "...",
         *           "rsuTopics": [
         *             {
         *               "topics": [{"name": "...", "selected": true/false}, ...],
         *               "rsu": {"ip": "...", "port": ...}
         *             }
         *           ],
         *           "timestamp": 1234567890
         *         }
         */
        Json::Value toJson() const
        {
            Json::Value json;
            
            json["unitId"] = _unitId;
            
            // Convert RSU topics to JSON array
            Json::Value rsuTopicsArray(Json::arrayValue);
            for (const auto& rsuTopic : _rsuTopics)
            {
                rsuTopicsArray.append(rsuTopic.toJson());
            }
            json["rsuTopics"] = rsuTopicsArray;
            
            json["timestamp"] = static_cast<Json::Int64>(_timestamp);
            
            return json;
        }

        /**
         * @brief Create TRUTopicsMessage from JSON object
         * @param json JSON object containing TRU topics data
         * @return TRUTopicsMessage instance
         */
        static TRUTopicsMessage fromJson(const Json::Value& json)
        {
            TRUTopicsMessage message;
            
            // Parse unit ID
            if (json.isMember("unitId") && json["unitId"].isString())
            {
                message.setUnitId(json["unitId"].asString());
            }

            //Parse Timestamp
            if (json.isMember("timestamp") && json["timestamp"].isNumeric())
            {
                message.setTimestamp(json["timestamp"].asInt64());
            }
            else if (json.isMember("timestamp") && json["timestamp"].isString()){
                message.setTimestamp(std::stoll( json["timestamp"].asString()));
            }
            
            // Parse RSU topics array
            if (json.isMember("rsuTopics") && json["rsuTopics"].isArray())
            {
                const Json::Value& rsuTopicsArray = json["rsuTopics"];
                for (const auto& rsuTopicJson : rsuTopicsArray)
                {
                    // Parse each RSUTopicsMessage
                    RSUTopicsMessage rsuTopic;
                    
                    // Parse topics array
                    if (rsuTopicJson.isMember("topics") && rsuTopicJson["topics"].isArray())
                    {
                        const Json::Value& topicsArray = rsuTopicJson["topics"];
                        std::vector<TopicMessage> topics;
                        for (const auto& topicJson : topicsArray)
                        {
                            TopicMessage topic;
                            if (topicJson.isMember("name") && topicJson["name"].isString())
                            {
                                topic.setName(topicJson["name"].asString());
                            }
                            if (topicJson.isMember("selected") && topicJson["selected"].isBool())
                            {
                                topic.setSelected(topicJson["selected"].asBool());
                            }
                            topics.push_back(topic);
                        }
                        rsuTopic.setTopics(topics);
                    }
                    
                    // Parse RSU endpoint
                    if (rsuTopicJson.isMember("rsu") && rsuTopicJson["rsu"].isObject())
                    {
                        const Json::Value& rsuEndpointJson = rsuTopicJson["rsu"];
                        rsuEndpoint endpoint;
                        
                        if (rsuEndpointJson.isMember("ip") && rsuEndpointJson["ip"].isString())
                        {
                            endpoint.ip = rsuEndpointJson["ip"].asString();
                        }
                        if (rsuEndpointJson.isMember("port") && rsuEndpointJson["port"].isInt())
                        {
                            endpoint.port = rsuEndpointJson["port"].asInt();
                        }
                        
                        rsuTopic.setRsuEndpoint(endpoint);
                    }
                    
                    message.addRsuTopic(rsuTopic);
                }
            }
            
            // Parse timestamp
            if (json.isMember("timestamp") && json["timestamp"].isInt64())
            {
                message.setTimestamp(json["timestamp"].asInt64());
            }
            
            return message;
        }

        /**
         * @brief Validate that the message has required fields
         * @return bool True if unitId is not blank/empty
         */
        bool isValid() const
        {
            return !_unitId.empty();
        }

        /**
         * @brief Convert TRUTopicsMessage to JSON string
         * @return std::string JSON string representation
         */
        std::string toString() const
        {
            Json::FastWriter writer;
            return writer.write(toJson());
        }
    };

} // namespace TelematicBridge
