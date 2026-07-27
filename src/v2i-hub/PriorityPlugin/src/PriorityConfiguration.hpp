/**
 * Copyright (C) 2026 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "PriorityConfigurationException.hpp"

namespace PriorityPlugin {

    static constexpr const char *IntersectionIDKey = "IntersectionID";
    static constexpr const char *IPKey             = "IP";
    static constexpr const char *PortKey           = "Port";
    static constexpr const char *LaneKey           = "Lane";
    static constexpr const char *StrategyKey       = "Strategy";

    static constexpr int    StrategyMin             = 1;
    static constexpr int    StrategyMax             = 255;
    static constexpr size_t ReserviceClassTimesSize = 10;

    /**
     * @brief Operating role of the plugin per NTCIP 1211: Priority Request
     *        Server (PRS) or Priority Request Generator (PRG).
     */
    enum class PluginRole { PRS, PRG };

    /**
     * @brief Returns the configuration string for a PluginRole.
     */
    constexpr const char *pluginRoleToString(PluginRole role) {
        return role == PluginRole::PRS ? "PRS" : "PRG";
    }

    /**
     * @brief Parsed TSC controller entry from the TSC_Configuration_List JSON.
     */
    struct ControllerConfig {
        long        intersectionID = 0;
        std::string ip;
        uint16_t    port = 0;
    };

    /**
     * @brief Parsed lane-to-strategy entry from the LaneStrategyMapping JSON.
     */
    struct LaneStrategyEntry {
        long    intersectionID = 0;
        long    lane           = 0;
        uint8_t strategy       = 0;
    };

    /**
     * @brief Parses the TSC_Configuration_List JSON array.
     * @param json The raw JSON array string.
     * @return std::vector<ControllerConfig> One entry per JSON object in the array.
     * @throws PriorityConfigurationException If the string is empty.
     */
    std::vector<ControllerConfig> parseTscConfigurationList(const std::string &json);

    /**
     * @brief Parses the LaneStrategyMapping JSON array.
     * @param json The raw JSON array string.
     * @return std::vector<LaneStrategyEntry> One entry per accepted JSON object.
     * @throws PriorityConfigurationException If the string is empty.
     */
    std::vector<LaneStrategyEntry> parseLaneStrategyMapping(const std::string &json);

    /**
     * @brief Parses a comma-separated list of unsigned integers into a fixed-size array
     *        of reservice periods (seconds), one per vehicle class type (1..10).
     * @param reserviceStr The comma-separated string.
     * @return std::array<uint32_t, 10> Reservice periods indexed by (classType - 1).
     * @throws PriorityConfigurationException If the string is empty.
     */
    std::array<uint32_t, ReserviceClassTimesSize> parseReserviceClassTimes(const std::string &reserviceStr);

    /**
     * @brief Parses the PluginRole configuration value.
     * @param role The configured role string. Must be exactly "PRS" or "PRG".
     * @return PluginRole The parsed role.
     * @throws PriorityConfigurationException If the string is not "PRS" or "PRG".
     */
    PluginRole parsePluginRole(const std::string &role);

} /* namespace PriorityPlugin */
