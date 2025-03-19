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
#include <SNMPClient.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <rsu/NTCIP_1218_MIB.h>
#include <rsu/RSU_MIB_4_1.h>
#include <PluginLog.h>
#include "ImmediateForwardConfiguration.h"

namespace ImmediateForward {
    
    /**
     * @brief Clear the immediate forward table on the RSU
     * @param client The SNMP client to use for the operation
     */
    void clearImmediateForwardTable(const std::unique_ptr<tmx::utils::snmp_client> &client);

    void setRSUMode(unsigned int mode, const std::unique_ptr<tmx::utils::snmp_client> &client);
    /**
     * @brief Initialize the immediate forward table on the RSU
     * @param client The SNMP client to use for the operation
     * @param messages The message configurations to add to the table
     */
    std::unordered_map<std::string, unsigned int> initializeImmediateForwardTable(const std::unique_ptr<tmx::utils::snmp_client> &client, const std::vector<Message> &messages);

    /**
     * @brief Send an NTCIP 1218 message to the RSU
     * @param client The SNMP client to use for the operation
     * @param message The IMF message to send
     * @param index The index of the message in the table
     */
    void sendNTCIP1218ImfMessage(const std::unique_ptr<tmx::utils::snmp_client> &client, const std::string &message, unsigned int index);

}
