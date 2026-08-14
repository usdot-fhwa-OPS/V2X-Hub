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
#include <tmx/messages/byte_stream.hpp>

namespace ImmediateForward
{
    /**
     * @brief Read the RawSpdu payload back out of a received TMX message.
     *
     * The payload of a RawSpdu message is a JSON object rather than a hex string, so it cannot be
     * read through IvpMessage::payload::valuestring the way J2735 payloads are.
     *
     * @param ivpMsg The received message. Its contents are copied, not taken over.
     * @return tmx::messages::RawSpdu
     * @throws tmx::TmxException if the message is null, carries no SPDU bytes, or carries a J2735
     * payload whose type could not be identified.
     */
    tmx::messages::RawSpdu getRawSpdu(IvpMessage *ivpMsg);

    /**
     * @brief Hex encode bytes in upper case, the form the immediate forward protocols expect.
     */
    std::string toUpperHex(const tmx::byte_stream &bytes);
}
