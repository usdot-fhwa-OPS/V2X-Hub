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
#include "SpduForwarder.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

#include <tmx/messages/byte_stream.hpp>
#include <tmx/messages/routeable_message.hpp>

namespace ImmediateForward
{
    namespace
    {
        // Value RawSpduUtils records when the J2735 payload cannot be identified. Duplicated here so
        // that this plugin does not have to pull in the 1609.2 decoding dependency.
        constexpr const char *UnknownMessageType = "Unknown";

        /**
         * Strip an optional "0x" prefix and any leading zeros, and upper case the rest, so that two
         * PSIDs written differently in configuration and on the wire can be compared directly.
         */
        std::string normalizePsid(const std::string &psid)
        {
            std::string normalized = psid;
            if (normalized.size() > 1 && normalized[0] == '0' &&
                    (normalized[1] == 'x' || normalized[1] == 'X'))
            {
                normalized.erase(0, 2);
            }

            const auto firstSignificant = normalized.find_first_not_of('0');
            if (firstSignificant == std::string::npos)
            {
                // The PSID is all zeros (or empty).
                return "0";
            }
            normalized.erase(0, firstSignificant);

            std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                    [](unsigned char c){ return std::toupper(c); });
            return normalized;
        }
    }

    std::string toPsidHex(int psid)
    {
        std::ostringstream os;
        os << "0x" << std::uppercase << std::hex << psid;
        return os.str();
    }

    bool psidMatches(const std::string &psidA, const std::string &psidB)
    {
        return normalizePsid(psidA) == normalizePsid(psidB);
    }

    SpduForwardData extractSpduForwardData(IvpMessage *ivpMsg)
    {
        if (ivpMsg == nullptr)
        {
            throw tmx::TmxException("Cannot extract SPDU data from a null message.");
        }

        // routeable_message copies the IvpMessage contents, so the caller keeps ownership of ivpMsg.
        tmx::routeable_message rMsg(ivpMsg);
        tmx::messages::RawSpdu rawSpdu = rMsg.get_payload<tmx::messages::RawSpdu>();

        const tmx::byte_stream fullByteData = rawSpdu.get_fullByteData();
        if (fullByteData.empty())
        {
            throw tmx::TmxException("SPDU message contains no fullByteData to forward.");
        }

        const std::string messageType = rawSpdu.get_messageType();
        if (messageType.empty() || messageType == UnknownMessageType)
        {
            throw tmx::TmxException("SPDU message carries an unidentified J2735 payload type.");
        }

        SpduForwardData data;
        // byte_stream_encode writes lowercase hex, while the RSU immediate forward payloads this
        // plugin sends are uppercase.
        data.payloadHex = tmx::byte_stream_encode(fullByteData);
        std::transform(data.payloadHex.begin(), data.payloadHex.end(), data.payloadHex.begin(),
                [](unsigned char c){ return std::toupper(c); });
        data.psidHex = toPsidHex(rawSpdu.get_psid());
        data.messageType = messageType;

        return data;
    }
}
