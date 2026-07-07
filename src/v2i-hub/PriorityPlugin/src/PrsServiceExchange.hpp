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

#include "SNMPClient.h"
#include "PriorityRequestProcessor.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <carma-clock/carma_clock.h>

namespace PriorityPlugin {

    /**
     * @brief References for a single PRS-CO service exchange.
     */
    struct ServiceExchangeContext {
        PriorityRequestProcessor &processor;            ///< NTCIP 1211 priority request processor
        std::mutex &tableMutex;                         ///< Guards the processor's table
        std::vector<uint8_t> &lastSentServiceRequest;   ///< Last prsServiceRequest payload SET to the CO (mutated on a successful SET)
        bool &prsBusy;                                  ///< PRS busy flag, set True during prioritization
    };

    /**
     * @brief Outcome of a single PRS-CO service exchange iteration.
     */
    enum class ExchangeResult : uint8_t {
        Ok,             // Exchange completed (CO not busy; prioritization applied if needed).
        SnmpSetFailed,  // The prsServiceRequest SET to the CO failed.
        CoError,        // GET failed or the CO response could not be decoded.
        CoStillBusy     // CO remained busy after the retry budget; no prioritization this iteration.
    };

    /**
     * @brief Sends the encoded OCTET STRING to a TSC via SNMP SET.
     * @param client the SNMP client for the target controller
     * @param oidStr the NTCIP 1211 OID to SET
     * @param data the OER-encoded payload
     * @return true on success, false on failure or if the client is null.
     */
    bool SnmpSet(const std::shared_ptr<tmx::utils::snmp_client> &client, const std::string &oidStr, const std::vector<uint8_t> &data);

    /**
     * @brief Performs an SNMP GET on a given OID and returns the raw bytes.
     * @param client the SNMP client for the target controller
     * @param oidStr the NTCIP 1211 OID to GET
     * @param data filled with the response bytes on success
     * @return true on success, false on failure or if the client is null.
     */
    bool SnmpGet(const std::shared_ptr<tmx::utils::snmp_client> &client, const std::string &oidStr, std::vector<uint8_t> &data);

    /**
     * @brief Performs one iteration of the NTCIP 1211 4.2.4.1.2 PRS-CO exchange against a single controller.
     * @param ctx                  references to exchange state
     * @param targetClient         SNMP client for the selected controller
     * @param targetIntersectionID intersection ID of the selected controller
     * @param clock                clock used for CO re-polling and timestamps
     * @param running              loop-running flag; the GET-poll exits early if it clears
     * @return ExchangeResult describing how the iteration ended.
     */
    ExchangeResult DoOneServiceExchange(ServiceExchangeContext &ctx, const std::shared_ptr<tmx::utils::snmp_client> &targetClient, long targetIntersectionID, const std::shared_ptr<fwha_stol::lib::time::CarmaClock> &clock, const std::atomic<bool> &running);

} /* namespace PriorityPlugin */
