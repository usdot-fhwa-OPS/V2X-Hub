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

#include "PriorityConfiguration.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <PluginLog.h>

using namespace tmx::utils;

namespace PriorityPlugin {

    std::vector<ControllerConfig> parseTscConfigurationList(const std::string &json)
    {
        std::vector<ControllerConfig> configs;
        if (json.empty()) {
            return configs;
        }

        boost::property_tree::ptree pt;
        std::istringstream iss(json);
        boost::property_tree::read_json(iss, pt);

        for (const auto &[_, node] : pt) {
            ControllerConfig cfg;
            cfg.intersectionID = node.get<long>(IntersectionIDKey);
            cfg.ip             = node.get<std::string>(IPKey);
            cfg.port           = node.get<uint16_t>(PortKey);
            configs.push_back(std::move(cfg));
        }
        return configs;
    }

    std::vector<LaneStrategyEntry> parseLaneStrategyMapping(const std::string &json)
    {
        std::vector<LaneStrategyEntry> entries;
        if (json.empty()) {
            return entries;
        }

        boost::property_tree::ptree pt;
        std::istringstream iss(json);
        boost::property_tree::read_json(iss, pt);

        for (const auto &[_, node] : pt) {
            auto intID       = node.get<long>(IntersectionIDKey);
            auto lane        = node.get<long>(LaneKey);
            auto strategyVal = node.get<int>(StrategyKey);
            if (strategyVal < StrategyMin || strategyVal > StrategyMax) {
                PLOG(logWARNING)
                    << "LaneStrategyMapping Strategy must be " << StrategyMin << ".." << StrategyMax
                    << ", skipping entry: IntersectionID=" << intID
                    << " Lane=" << lane
                    << " Strategy=" << strategyVal;
                continue;
            }
            entries.push_back({intID, lane, static_cast<uint8_t>(strategyVal)});
        }
        return entries;
    }

    std::array<uint32_t, ReserviceClassTimesSize> parseReserviceClassTimes(const std::string &reserviceStr)
    {
        std::array<uint32_t, ReserviceClassTimesSize> result{};
        if (reserviceStr.empty()) {
            return result;
        }

        std::istringstream rss(reserviceStr);
        std::string tok;
        size_t idx = 0;
        while (std::getline(rss, tok, ',') && idx < ReserviceClassTimesSize) {
            try {
                result[idx] = static_cast<uint32_t>(std::stoul(tok));
            } catch (const std::invalid_argument &e) {
                PLOG(logWARNING) << "Invalid ReserviceClassTimes value at index " << idx
                                             << ": '" << tok << "' (" << e.what() << ")";
            } catch (const std::out_of_range &e) {
                PLOG(logWARNING) << "ReserviceClassTimes value out of range at index " << idx
                                             << ": '" << tok << "' (" << e.what() << ")";
            }
            idx++;
        }
        return result;
    }

} /* namespace PriorityPlugin */
