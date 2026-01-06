#pragma once
#include <tmx/messages/message.hpp>

namespace tmx::messages{
    struct TelematicRSUUnit
    {
        TelematicRSUUnit() {}

        TelematicRSUUnit(std::string unitId, int16_t maxConnections):
            unitId(unitId), maxConnections(maxConnections), bridgePluginHeartbeatInterval(10),
            healthMonitorPluginHeartbeatInterval(10), rsuStatusMonitorInterval(10) {}

        TelematicRSUUnit (std::string unitId, int16_t maxConnections, int16_t pluginHeartbeatInterval,
                int16_t healthMonitorInterval, int16_t rsuStatusInterval): unitId(unitId), maxConnections(maxConnections),
                bridgePluginHeartbeatInterval(pluginHeartbeatInterval), healthMonitorPluginHeartbeatInterval(healthMonitorInterval),
                rsuStatusMonitorInterval(rsuStatusInterval) {}

        static message_tree_type to_tree(const TelematicRSUUnit& unit)
        {
            message_tree_type tree;
            tree.put("unitId",unit.unitId);
            tree.put("maxConnections", unit.maxConnections);
            tree.put("bridgePluginHeartbeatInterval", unit.bridgePluginHeartbeatInterval);
            tree.put("healthMonitorPluginHeartbeatInterval", unit.healthMonitorPluginHeartbeatInterval);
            tree.put("rsuStatusMonitorInterval", unit.rsuStatusMonitorInterval);

            return tree;
        }

        static TelematicRSUUnit from_tree(const message_tree_type& tree)
        {
            TelematicRSUUnit unit;
            unit.unitId = tree.get<std::string>("unitId");
            unit.maxConnections = tree.get<int16_t>("maxConnections");
            unit.bridgePluginHeartbeatInterval = tree.get<int16_t>("bridgePluginHeartbeatInterval");
            unit.healthMonitorPluginHeartbeatInterval = tree.get<int16_t>("healthMonitorPluginHeartbeatInterval");
            unit.rsuStatusMonitorInterval = tree.get<int16_t>("rsuStatusMonitorInterval");

            return unit;
        }

        std::string unitId;
        int16_t maxConnections;
        int16_t bridgePluginHeartbeatInterval;
        int16_t healthMonitorPluginHeartbeatInterval;
        int16_t rsuStatusMonitorInterval;
    };
}