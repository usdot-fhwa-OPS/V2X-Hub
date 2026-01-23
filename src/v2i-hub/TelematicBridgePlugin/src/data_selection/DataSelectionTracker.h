#pragma once
#include <nats/nats.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <mutex>
#include "PluginLog.h"
#include "ThreadTimer.h"
#include "TelematicBridgeException.h"
#include "RSUConfigWorker.h"
#include "TelematicUnit.h"
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string.hpp>
#include "health_monitor/TRUHealthStatusTracker.h"
#include "health_monitor/RSUHealthStatusMessage.h"
#include "health_monitor/HealthStatusMessageMapper.h"
#include "data_selection/TRUTopicsMessage.h"

namespace TelematicBridge
{
    class DataSelectionTracker
    {
    private:        
        // Map to track available topics per RSU (key: "ip", value: set of topics)
        std::unordered_map<std::string, std::unordered_set<std::string>> _rsuAvailableTopicsMap;
        std::mutex _rsuAvailableTopicsMutex;
        
        // Map to track selected topics per RSU (key: "ip", value: set of selected topic names)
        std::unordered_map<std::string, std::unordered_set<std::string>> _rsuSelectedTopicsMap;
        std::mutex _rsuSelectedTopicsMutex;

        TRUTopicsMessage _latestSelectedTopicsMessage;
        TRUTopicsMessage _latestAvailableTopicsMessage;

    public:
        DataSelectionTracker() = default;
        ~DataSelectionTracker()=default;
        
        void updateRsuAvailableTopics(const std::string &rsuIp, int rsuPort, const std::string &topic, const std::string &unitId)
        {
            std::lock_guard<std::mutex> lock(_rsuAvailableTopicsMutex);
            _rsuAvailableTopicsMap[rsuIp].insert(topic);     
            _latestAvailableTopicsMessage.setCurrentTimestamp();
            _latestAvailableTopicsMessage.setUnitId(unitId);
            RSUTopicsMessage rsuTopicsMsg;
            rsuTopicsMsg.setRsuEndpoint({rsuIp, rsuPort});
            std::vector<TopicMessage> topics;
            auto it = _rsuAvailableTopicsMap.find(rsuIp);
            if (it != _rsuAvailableTopicsMap.end())
            {
                for (const auto& topicName : it->second)
                {
                    topics.emplace_back(topicName, false);
                }
            }
            rsuTopicsMsg.setTopics(topics);
            _latestAvailableTopicsMessage.addRsuTopic(rsuTopicsMsg);
        }

        std::string latestAvailableTopicsMessageToJsonString()
        {
            std::lock_guard<std::mutex> lock(_rsuAvailableTopicsMutex);
            return _latestAvailableTopicsMessage.toString();
        }

        /**
         * @brief Check if a topic is selected for a specific RSU
         * @param rsuIp RSU IP address
         * @param topic Topic name to check
         * @return true if the topic is selected for the specified RSU, false otherwise
         */
        bool inRsuSelectedTopics(const std::string &rsuIp, const std::string &topicName) 
        {
            // Use only RSU IP as the key (port is ignored)
            std::lock_guard<std::mutex> lock(_rsuSelectedTopicsMutex);
            
            // Check if this specific RSU has the topic selected
            auto it = _rsuSelectedTopicsMap.find(rsuIp);
            if (it != _rsuSelectedTopicsMap.end())
            {
                return it->second.find(topicName) != it->second.end();
            }
            
            return false;
        }

        void updateLatestSelectedTopics(const char* msgStr)
        {
            std::lock_guard<std::mutex> lock(_rsuSelectedTopicsMutex);
            Json::CharReaderBuilder reader;
            Json::Value root;
            std::string errs;
            std::istringstream s(msgStr);
            
            if (Json::parseFromStream(reader, s, &root, &errs))
            {
                auto incomingMsg = TRUTopicsMessage::fromJson(root);
                
                // Clear old selected topics for all RSUs
                {
                    _rsuSelectedTopicsMap.clear();
                }
                
                // Process selected topics from the incoming message
                for (const auto& rsuTopicsMsg : incomingMsg.getRsuTopics())
                {
                    const auto& endpoint = rsuTopicsMsg.getRsuEndpoint();
                    // Use only RSU IP as the key (port is ignored)
                    std::string rsuIp = endpoint.ip;
                    
                    for (const auto& topic : rsuTopicsMsg.getTopics())
                    {
                        if (topic.isSelected())
                        {
                            _rsuSelectedTopicsMap[rsuIp].insert(topic.getName());
                        }
                    }
                }

                incomingMsg.setCurrentTimestamp();
                _latestSelectedTopicsMessage = incomingMsg;
            }
            else
            {
                throw std::runtime_error("Failed to parse RSU selected topics JSON: " + errs);
            }
        }        

        std::string latestSelectedTopicsMessageToJsonString()
        {
            std::lock_guard<std::mutex> lock(_rsuSelectedTopicsMutex);
            return _latestSelectedTopicsMessage.toString();
        }
    };
    
    
} 
