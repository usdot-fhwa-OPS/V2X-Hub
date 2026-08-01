/**
 * Copyright (C) 2025 LEIDOS.
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

#include <string>

#include <RawSpdu.h>
#include <tmx/IvpMessage.h>
#include <tmx/TmxException.hpp>

namespace ImmediateForward
{
    /**
     * The pieces of a RawSpdu message needed to forward it to an RSU, formatted the
     * way the immediate forward protocols expect them.
     */
    struct SpduForwardData
    {
        /// Full SPDU bytes as an uppercase hex string, ready to use as an IMF payload.
        std::string payloadHex;
        /// PSID carried by the SPDU, formatted as "0x<HEX>" to match the configuration format.
        std::string psidHex;
        /// J2735 subtype of the unsecured payload, e.g. "BSM", "SPAT-P". Matched against tmxType.
        std::string messageType;
    };

    /**
     * @brief Pull the forwardable fields out of an IvpMessage carrying a RawSpdu payload.
     *
     * The payload of a RawSpdu message is a JSON object rather than a hex string, so it has to be
     * read back through routeable_message instead of IvpMessage::payload::valuestring.
     *
     * @param ivpMsg The received message. Its contents are copied, not taken over.
     * @throws tmx::TmxException if the SPDU bytes are missing or the J2735 type is unidentified.
     */
    SpduForwardData extractSpduForwardData(IvpMessage *ivpMsg);

    /**
     * @brief Compare two PSIDs written as hex strings, ignoring case, an optional "0x" prefix and
     * leading zeros, so that "0x0027" and "0x27" are considered equal.
     */
    bool psidMatches(const std::string &psidA, const std::string &psidB);

    /**
     * @brief Format a PSID as "0x<HEX>" using the same convention as the plugin configuration.
     */
    std::string toPsidHex(int psid);
}
