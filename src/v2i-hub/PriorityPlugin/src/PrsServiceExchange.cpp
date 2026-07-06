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

#include "PrsServiceExchange.hpp"

#include <array>

#include <tmx/messages/byte_stream.hpp>
#include <tsc/NTCIP_1211_MIB.h>

using namespace tmx::utils;

namespace PriorityPlugin {

    bool SnmpSet(const std::shared_ptr<snmp_client> &client, const std::string &oidStr, const std::vector<uint8_t> &data)
    {
        if (!client) {
            PLOG(logERROR) << "SNMP client not initialized for SET.";
            return false;
        }

        snmp_response_obj val;
        val.type = snmp_response_obj::response_type::STRING;
        val.val_string.assign(data.begin(), data.end());

        bool success = client->process_snmp_request(oidStr, request_type::SET, val);
        if (!success) {
            PLOG(logERROR) << "SNMP SET failed for OID: " << oidStr;
        }
        return success;
    }

    bool SnmpGet(const std::shared_ptr<snmp_client> &client, const std::string &oidStr, std::vector<uint8_t> &data)
    {
        if (!client) {
            PLOG(logERROR) << "SNMP client not initialized for GET.";
            return false;
        }

        snmp_response_obj val;
        val.type = snmp_response_obj::response_type::STRING;

        if (bool success = client->process_snmp_request(oidStr, request_type::GET, val);
            !success) {
            PLOG(logERROR) << "SNMP GET failed for OID: " << oidStr;
            return false;
        }

        data.assign(val.val_string.begin(), val.val_string.end());
        return true;
    }

    ExchangeResult DoOneServiceExchange(ServiceExchangeContext &ctx, const std::shared_ptr<snmp_client> &targetClient, long targetIntersectionID, const std::shared_ptr<fwha_stol::lib::time::CarmaClock> &clock, const std::atomic<bool> &running)
    {
        // b) PRS shall SET prsServiceRequest to the CO.
        // Note: steps c-e are actions on the CO side.
        // Only send the SET when the encoded table differs from what the CO already holds.
        std::vector<uint8_t> setData;
        {
            std::lock_guard lock(ctx.tableMutex);
            setData = ctx.processor.EncodeServiceRequest(ctx.prsBusy);
        }

        if (bool shouldSet = (setData != ctx.lastSentServiceRequest);
            shouldSet) {
            PLOG(logDEBUG3) << "PRS SET prsServiceRequest to CO: " << tmx::byte_stream_encode(setData);
            if (bool setOk = SnmpSet(targetClient, tsc::mib::ntcip1211::NTCIP1211_PRS_SERVICE_REQUEST_OID, setData);
                !setOk) {
                PLOG(logERROR) << "PRS failed to SET prsServiceRequest to CO";
                return ExchangeResult::SnmpSetFailed;
            }
            ctx.lastSentServiceRequest = setData;
        }

        // f) PRS shall then send a GET prsServiceRequest to the CO.
        // Note: step g is on the CO side.
        // h) If coBusy is True, keep polling GET until False
        bool coBusy = true;
        bool coError = false;
        std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;
        int maxRetries = 3; // Limit to avoid infinite loop
        while (coBusy && running && maxRetries > 0) {
            --maxRetries;
            std::vector<uint8_t> getData;
            if (bool getOk = SnmpGet(targetClient, tsc::mib::ntcip1211::NTCIP1211_PRS_SERVICE_REQUEST_OID, getData);
                !getOk) {
                PLOG(logERROR) << "PRS failed to GET prsServiceRequest from CO";
                coError = true;
            }

            if (!coError && !PriorityRequestProcessor::DecodeCoServiceResponse(getData, coRows, coBusy)) {
                PLOG(logERROR) << "Failed to decode CO service response";
                coError = true;
            }

            if (coError) {
                break;
            }

            if (coBusy) {
                PLOG(logDEBUG1) << "coBusy is True, re-polling CO...";
                clock->sleep_for(10);
            }
        }

        if (coError) {
            return ExchangeResult::CoError;
        }

        for (size_t i = 0; i < coRows.size(); ++i) {
            const auto &[strategy, tsd, ted, status] = coRows[i];
            if (strategy == 0 && tsd == 0 && ted == 0 &&
                status == RequestStatus::idleNotValid) {
                continue;
            }
            PLOG(logDEBUG1) << "CO row[" << i << "]: IntersectionID=" << targetIntersectionID
                            << " strategy=" << static_cast<int>(strategy)
                            << " TSD=" << tsd << " TED=" << ted
                            << " status=" << static_cast<int>(status);
        }

        if (coBusy) {
            return ExchangeResult::CoStillBusy;
        }

        // i) coBusy is False: set prsBusy to True and perform prioritization
        std::vector<uint8_t> updatedSetData;
        {
            std::lock_guard lock(ctx.tableMutex);

            auto now = static_cast<uint32_t>(clock->nowInSeconds());
            ctx.processor.ApplyCoStatusUpdates(coRows, now);

            ctx.prsBusy = true;
            ctx.processor.RunPrioritizationProcessing(now);

            // j) Upon completing prioritization, set prsBusy to False
            ctx.prsBusy = false;
            updatedSetData = ctx.processor.EncodeServiceRequest(ctx.prsBusy);
        }

        // j) SET prsServiceRequest to the CO only if the post-prioritization
        // table differs from what the CO already holds.
        if (updatedSetData != ctx.lastSentServiceRequest) {
            if (!SnmpSet(targetClient, tsc::mib::ntcip1211::NTCIP1211_PRS_SERVICE_REQUEST_OID, updatedSetData)) {
                PLOG(logERROR) << "PRS failed to SET prsServiceRequest to CO after prioritization";
                return ExchangeResult::SnmpSetFailed;
            }
            ctx.lastSentServiceRequest = updatedSetData;
        }

        return ExchangeResult::Ok;
    }

} /* namespace PriorityPlugin */
