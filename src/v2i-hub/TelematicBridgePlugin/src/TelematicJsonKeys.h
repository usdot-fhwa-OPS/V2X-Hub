#pragma once

namespace TelematicBridge
{
    namespace TelematicJsonKeys
    {
        // Unit Health Status Keys
        constexpr const char* UNIT_ID = "unitId";
        constexpr const char* BRIDGE_PLUGIN_STATUS = "bridgePluginStatus";
        constexpr const char* LAST_UPDATED_TIMESTAMP = "lastUpdatedTimestamp";
        
        // RSU Health Status Keys
        constexpr const char* RSU = "rsu";
        constexpr const char* RSU_IP = "ip";
        constexpr const char* RSU_PORT = "port";
        constexpr const char* STATUS = "status";
        constexpr const char* EVENT = "event";
        
        // TRU Health Status Keys
        constexpr const char* RSU_CONFIGS = "rsuConfigs";
        constexpr const char* UNIT_CONFIG = "unitConfig";
        constexpr const char* TIMESTAMP = "timestamp";
        
        // Data Selection Keys
        constexpr const char* RSU_TOPICS = "rsuTopics";
        constexpr const char* TOPICS = "topics";
        constexpr const char* TOPIC_NAME = "name";
        constexpr const char* TOPIC_SELECTED = "selected";

        constexpr const char* METADATA = "metadata";
        constexpr const char* PAYLOAD = "payload";
        constexpr const char* TOPIC_NAME_METADATA = "topicName";
        
        // RSU Health Message Mapper Keys (from external RSU status messages)
        constexpr const char* RSU_IP_ADDRESS = "rsuIpAddress";
        constexpr const char* RSU_SNMP_PORT = "rsuSnmpPort";
        constexpr const char* RSU_MODE = "rsuMode";
    }
}
