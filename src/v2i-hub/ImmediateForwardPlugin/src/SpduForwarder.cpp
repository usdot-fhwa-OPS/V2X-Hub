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

#include <tmx/messages/routeable_message.hpp>

namespace ImmediateForward
{
    tmx::messages::RawSpdu getRawSpdu(IvpMessage *ivpMsg)
    {
        if (ivpMsg == nullptr)
        {
            throw tmx::TmxException("Cannot read a RawSpdu from a null message.");
        }

        // routeable_message copies the IvpMessage contents, so the caller keeps ownership of ivpMsg.
        tmx::routeable_message rMsg(ivpMsg);
        tmx::messages::RawSpdu rawSpdu = rMsg.get_payload<tmx::messages::RawSpdu>();

        if (rawSpdu.get_fullByteData().empty())
        {
            throw tmx::TmxException("SPDU message contains no fullByteData to forward.");
        }

        // getJ2735SubType records UNKNOWN_SUBTYPE when it cannot identify the unsecured payload, and
        // there is no tmxType such a message could be routed to.
        const std::string messageType = rawSpdu.get_messageType();
        if (messageType.empty() || messageType == UNKNOWN_SUBTYPE)
        {
            throw tmx::TmxException("SPDU message carries an unidentified J2735 payload type.");
        }

        return rawSpdu;
    }

    std::string toUpperHex(const tmx::byte_stream &bytes)
    {
        // byte_stream_encode writes lowercase hex, while the RSU immediate forward payloads this
        // plugin sends are uppercase.
        std::string hex = tmx::byte_stream_encode(bytes);
        std::transform(hex.begin(), hex.end(), hex.begin(),
                [](unsigned char c){ return std::toupper(c); });
        return hex;
    }

}
