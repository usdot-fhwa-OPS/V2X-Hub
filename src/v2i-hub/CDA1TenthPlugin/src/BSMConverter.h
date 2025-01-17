/**
 * Copyright (C) 2024 LEIDOS.
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

#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>

using namespace tmx::messages;
using pt = boost::property_tree::ptree;
using std::string;
namespace CDA1TenthPlugin
{
  class BSMConverter
  {
  public:
    BSMConverter() = default;
    ~BSMConverter() = default;
    /***
     * @brief Convert a BasicSafetyMessage_t to a JSON ptree
     * @param bsm BasicSafetyMessage_t
     * @return JSON ptree
     */
    static pt toTree(const BasicSafetyMessage_t &bsm);
    /**
     * @brief Convert a JSON ptree to a JSON string
     * @param tree JSON ptree
     * @return BSM string in JSON format
     */
    static string toJsonString(const pt &tree);
    /**
     * @brief Convert TemporaryID_t to string
     * @param id TemporaryID_t
     * @return string
     */
    static string toTemporaryIdString(const TemporaryID_t &id);
    /**
     * @brief Convert TransmissionState_t to string
     * @param transmission TransmissionState_t
     * @return string
     */
    static string toTransmissionString(const TransmissionState_t &transmission);
    /**
     * @brief Convert TractionControlStatus_t to string
     * @param traction TractionControlStatus_t
     * @return string
     */
    static string toTractionString(const TractionControlStatus_t &traction);
    /**
     * @brief Converts the AuxiliaryBrakeStatus_t enum to a string representation.
     *
     * @param auxBrakes The AuxiliaryBrakeStatus_t enum value to be converted.
     * @return A string representation of the auxiliary brake status.
     */
    static string toAuxiliaryBrakeStatusString(const AuxiliaryBrakeStatus_t &auxBrakes);

    /**
     * @brief Converts the BrakeAppliedStatus_t enum to a string representation.
     *
     * @param brakeStatus The BrakeAppliedStatus_t enum value to be converted.
     * @return A string representation of the brake applied status.
     */
    static string toBrakeAppliedStatusString(const BrakeAppliedStatus_t &brakeStatus);

    /**
     * @brief Converts the AntiLockBrakeStatus_t enum to a string representation.
     *
     * @param abs The AntiLockBrakeStatus_t enum value to be converted.
     * @return A string representation of the brake system status.
     */
    static string toAntiLockBrakeStatusString(const AntiLockBrakeStatus_t &abs);

    /**
     * @brief Converts the StabilityControlStatus_t enum to a string representation.
     *
     * @param stabilityControl The StabilityControlStatus_t enum value to be converted.
     * @return A string representation of the stability control status.
     */
    static string toStabilityControlStatusString(const StabilityControlStatus_t &scs);

    /**
     * @brief Converts the BrakeBoostApplied_t enum to a string representation.
     *
     * @param brakeBoost The BrakeBoostApplied_t enum value to be converted.
     * @return A string representation of the brake boost applied status.
     */
    static string toBrakeBoostAppliedString(const BrakeBoostApplied_t &brakeBoost);
  };
} // namespace CDA1TenthPlugin
