#pragma once
#include <string>
#include <vector>
#include <jsoncpp/json/json.h>
#include "RSUConfigWorker.h"
#include "TopicMessage.h"

namespace TelematicBridge
{
    /**
     * @brief RSU Topics Message containing topics list and RSU endpoint
     */
    class RSUTopicsMessage
    {
    private:
        std::vector<TopicMessage> _topics;
        rsuEndpoint _rsuEndpoint;

    public:
        RSUTopicsMessage() = default;

        RSUTopicsMessage(const std::vector<TopicMessage>& topics, const rsuEndpoint& endpoint)
            : _topics(topics), _rsuEndpoint(endpoint) {}

        /**
         * @brief Get the topics list
         * @return const std::vector<TopicMessage>& Reference to topics vector
         */
        const std::vector<TopicMessage>& getTopics() const { return _topics; }

        /**
         * @brief Set the topics list
         * @param topics Vector of TopicMessage objects
         */
        void setTopics(const std::vector<TopicMessage>& topics) { _topics = topics; }

        /**
         * @brief Add a topic to the list
         * @param topic TopicMessage to add
         */
        void addTopic(const TopicMessage& topic) { _topics.push_back(topic); }

        /**
         * @brief Get the RSU endpoint
         * @return const rsuEndpoint& Reference to RSU endpoint
         */
        const rsuEndpoint& getRsuEndpoint() const { return _rsuEndpoint; }

        /**
         * @brief Set the RSU endpoint
         * @param endpoint RSU endpoint to set
         */
        void setRsuEndpoint(const rsuEndpoint& endpoint) { _rsuEndpoint = endpoint; }

        /**
         * @brief Convert RSUTopicsMessage to JSON object
         * @return Json::Value JSON representation with structure:
         *         {
         *           "topics": [{"name": "...", "selected": true/false}, ...],
         *           "rsuEndpoint": {"ip": "...", "port": ...}
         *         }
         */
        Json::Value toJson() const
        {
            Json::Value json;
            
            // Convert topics to JSON array
            Json::Value topicsArray(Json::arrayValue);
            for (const auto& topic : _topics)
            {
                topicsArray.append(topic.toJson());
            }
            json["topics"] = topicsArray;
            
            // Convert RSU endpoint to JSON
            Json::Value rsuJson;
            rsuJson["ip"] = _rsuEndpoint.ip;
            rsuJson["port"] = _rsuEndpoint.port;
            json["rsuEndpoint"] = rsuJson;
            
            return json;
        }
    };

} // namespace TelematicBridge
