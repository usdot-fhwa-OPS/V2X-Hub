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

#include "PriorityPlugin.hpp"
#include <map>

using namespace tmx::messages;
using namespace tmx::utils;

namespace PriorityPlugin {

    void PriorityPlugin::ServiceExchangeLoop()
    {
        PLOG(logINFO) << "PRS service exchange loop started, poll interval = " << _pollIntervalMs << "ms";

        while (_running) {
            // Determine which controller to communicate with.
            // Use the first controller that has any active/ready request in the table.
            std::shared_ptr<snmp_client> targetClient;
            {
                std::lock_guard lock(_tableMutex);
                for (const auto &entry : _processor.Table()) {
                    if (entry.statusInPRS == RequestStatus::idleNotValid) {
                        continue;
                    }
                    auto it = _controllers.find(entry.intersectionID);
                    if (it != _controllers.end() && it->second.snmpClient) {
                        targetClient = it->second.snmpClient;
                        break;
                    }
                }
                // If no active requests, use first available controller (idle polling)
                if (!targetClient && !_controllers.empty()) {
                    targetClient = _controllers.begin()->second.snmpClient;
                }
            }

            if (!targetClient) {
                std::this_thread::sleep_for(std::chrono::milliseconds(_pollIntervalMs));
                continue;
            }

            // b) PRS shall SET prsServiceRequest to the CO.
            // Note: steps c-e are actions on the CO side.
            std::vector<uint8_t> setData;
            {
                std::lock_guard lock(_tableMutex);
                setData = _processor.EncodeServiceRequest(_prsBusy);
            }

            PLOG(logDEBUG3) << "PRS SET prsServiceRequest to CO: " << tmx::byte_stream_encode(setData);
            if (bool setOk = SnmpSet(targetClient, NTCIP1211_PRS_SERVICE_REQUEST_OID, setData);
                !setOk) {
                PLOG(logERROR) << "PRS failed to SET prsServiceRequest to CO";
                std::this_thread::sleep_for(std::chrono::milliseconds(_pollIntervalMs));
                continue;
            }

            // f) PRS shall then send a GET prsServiceRequest to the CO.
            // Note: step g is on the CO side.
            // h) If coBusy is True, keep polling GET until False
            bool coBusy = true;
            bool coError = false;
            std::array<CoServiceResponseRow, MAX_SERVICE_REQUESTS> coRows;
            int maxRetries = 3; // Limit to avoid infinite loop
            while (coBusy && _running && maxRetries > 0) {
                --maxRetries;
                std::vector<uint8_t> getData;
                if (bool getOk = SnmpGet(targetClient, NTCIP1211_PRS_SERVICE_REQUEST_OID, getData);
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
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }

            for (size_t i = 0; !coError && i < coRows.size(); ++i) {
                const auto &[strategy, tsd, ted, status] = coRows[i];
                if (strategy == 0 && tsd == 0 && ted == 0 &&
                    status == RequestStatus::idleNotValid) {
                    continue;
                }
                PLOG(logDEBUG1) << "CO row[" << i << "]: strategy=" << static_cast<int>(strategy)
                                << " TSD=" << tsd << " TED=" << ted
                                << " status=" << static_cast<int>(status);
            }

            // i) coBusy is False: set prsBusy to True and perform prioritization
            if (!coBusy) {
                std::vector<uint8_t> updatedSetData;
                {
                    std::lock_guard lock(_tableMutex);

                    auto now = static_cast<uint32_t>(std::time(nullptr));
                    _processor.ApplyCoStatusUpdates(coRows, now);

                    _prsBusy = true;
                    _processor.RunPrioritizationProcessing(now);

                    // j) Upon completing prioritization, set prsBusy to False
                    _prsBusy = false;
                    updatedSetData = _processor.EncodeServiceRequest(_prsBusy);
                }

                // j) SET prsServiceRequest to the CO with updated table and prsBusy=False
                if (!SnmpSet(targetClient, NTCIP1211_PRS_SERVICE_REQUEST_OID, updatedSetData)) {
                    PLOG(logERROR) << "PRS failed to SET prsServiceRequest to CO after prioritization";
                }
            }

            _serviceExchanges++;
            if (_serviceExchanges % 10 == 0) {
                SetStatus(_keyServiceExchanges, _serviceExchanges);
            }

            // Broadcast SSM reflecting current table state
            BroadcastSSMFromTable();

            std::this_thread::sleep_for(std::chrono::milliseconds(_pollIntervalMs));
        }

        PLOG(logINFO) << "PRS service exchange loop stopped.";
    }

    bool PriorityPlugin::SnmpSet(const std::shared_ptr<snmp_client> &client, const std::string &oidStr, const std::vector<uint8_t> &data) const
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

    bool PriorityPlugin::SnmpGet(const std::shared_ptr<snmp_client> &client, const std::string &oidStr, std::vector<uint8_t> &data) const
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

    long PriorityPlugin::MapNTCIPstatusToSSM(RequestStatus status) const
    {
        switch (status) {
            case RequestStatus::readyQueued:
                return PrioritizationResponseStatus_requested;
            case RequestStatus::activeProcessing:
            case RequestStatus::activeAdjustNotNeeded:
            case RequestStatus::activeOverride:
                return PrioritizationResponseStatus_processing;
            case RequestStatus::closedCompleted:
                return PrioritizationResponseStatus_granted;
            case RequestStatus::closedCanceled:
            case RequestStatus::closedTimerError:
            case RequestStatus::closedStrategyError:
            case RequestStatus::closedFlash:
            case RequestStatus::activeNotOverridden:
            case RequestStatus::readyOverridden:
                return PrioritizationResponseStatus_rejected;
            case RequestStatus::closedTimeToLiveError:
                return PrioritizationResponseStatus_maxPresence;
            case RequestStatus::reserviceError:
                return PrioritizationResponseStatus_reserviceLocked;
            default:
                return PrioritizationResponseStatus_unknown;
        }
    }

    void PriorityPlugin::BroadcastSSMFromTable()
    {
        // Collect non-idle entries grouped by intersection, skipping entries
        // that have already been broadcast the maximum number of times for
        // their current status.
        std::map<long, std::vector<PriorityRequestEntry *>> byIntersection;
        {
            std::lock_guard lock(_tableMutex);
            for (auto &entry : _processor.Table()) {
                if (entry.statusInPRS == RequestStatus::idleNotValid) continue;
                if (entry.statusInPRS != entry.ssmLastStatus) {
                    entry.ssmBroadcastCount = 0;
                    entry.ssmLastStatus = entry.statusInPRS;
                }
                if (_maxSsmBroadcastsPerStatus > 0 && entry.ssmBroadcastCount >= _maxSsmBroadcastsPerStatus) continue;
                entry.ssmBroadcastCount++;
                byIntersection[entry.intersectionID].push_back(&entry);
            }
        }

        if (byIntersection.empty()) return;

        auto ssmPtr = std::make_shared<SignalStatusMessage_t>();
        memset(ssmPtr.get(), 0, sizeof(SignalStatusMessage_t));

        time_t nowEpoch = std::time(nullptr);
        struct tm utcNow;
        gmtime_r(&nowEpoch, &utcNow);
        auto timeStamp = (MinuteOfTheYear_t *)calloc(1, sizeof(MinuteOfTheYear_t));
        *timeStamp = static_cast<MinuteOfTheYear_t>(utcNow.tm_yday * 24 * 60 + utcNow.tm_hour * 60 + utcNow.tm_min);
        ssmPtr->timeStamp = timeStamp;
        ssmPtr->second = static_cast<DSecond_t>(utcNow.tm_sec * 1000);

        // ssmPtr->sequenceNumber increments for every new SSM broadcast
        _ssmSequenceCounter++;
        auto msgSequenceNumber = (Common_MsgCount_t *)calloc(1, sizeof(Common_MsgCount_t));
        *msgSequenceNumber = _ssmSequenceCounter;
        ssmPtr->sequenceNumber = msgSequenceNumber;

        for (const auto &pair : byIntersection) {
            long intID = pair.first;
            const auto &entries = pair.second;

            // Create a key to track the package contents for this intersection
            // and detect changes and increment signalStatus->sequenceNumber accordingly
            std::ostringstream seqKey;
            for (const auto *entry : entries) {
                seqKey << static_cast<int>(entry->requestID) << ","
                   << static_cast<int>(entry->statusInPRS) << ","
                   << static_cast<int>(entry->sequenceNumber) << ","
                   << entry->timeOfServiceDesiredInPRS << ","
                   << entry->timeOfEstimatedDepartureInPRS << ","
                   << static_cast<int>(entry->inboundPresent) << ","
                   << entry->inboundValue << ";";
            }
            std::string currentSeqKey = seqKey.str();

            auto lastSeqKeyIter = _lastSignalStatusKey.find(intID);
            if (lastSeqKeyIter == _lastSignalStatusKey.end() || lastSeqKeyIter->second != currentSeqKey) {
                _signalStatusSeqByIntersection[intID]++;
                _lastSignalStatusKey[intID] = currentSeqKey;
            }

            SignalStatus *signalStatus = (SignalStatus *)calloc(1, sizeof(SignalStatus));
            signalStatus->id.id = intID;
            signalStatus->sequenceNumber = _signalStatusSeqByIntersection[intID];

            for (const auto *entry : entries) {
                SignalStatusPackage *pkg = (SignalStatusPackage *)calloc(1, sizeof(SignalStatusPackage));
                pkg->requester = (SignalRequesterInfo *)calloc(1, sizeof(SignalRequesterInfo));
                pkg->requester->request = entry->requestID;
                // requester->sequenceNumber is the SRM sequence number being responded to
                pkg->requester->sequenceNumber = entry->sequenceNumber;

                if (entry->vehicleID.size() == sizeof(StationID_t)) {
                    StationID_t stationID = 0;
                    std::memcpy(&stationID, entry->vehicleID.data(), sizeof(StationID_t));
                    pkg->requester->id.choice.stationID = stationID;
                    pkg->requester->id.present = VehicleID_PR_stationID;
                }
                else if (!entry->vehicleID.empty()) {
                    pkg->requester->id.choice.entityID.buf = (uint8_t *)calloc(entry->vehicleID.size(), 1);
                    std::memcpy(pkg->requester->id.choice.entityID.buf,
                                entry->vehicleID.data(), entry->vehicleID.size());
                    pkg->requester->id.choice.entityID.size = entry->vehicleID.size();
                    pkg->requester->id.present = VehicleID_PR_entityID;
                }

                auto rolePtr = (BasicVehicleRole_t *)calloc(1, sizeof(BasicVehicleRole_t));
                *rolePtr = static_cast<BasicVehicleRole_t>(entry->role);
                pkg->requester->role = rolePtr;

                pkg->inboundOn.present = static_cast<IntersectionAccessPoint_PR>(entry->inboundPresent);
                if (entry->inboundPresent == IntersectionAccessPoint_PR_lane)
                    pkg->inboundOn.choice.lane = entry->inboundValue;
                else if (entry->inboundPresent == IntersectionAccessPoint_PR_approach)
                    pkg->inboundOn.choice.approach = entry->inboundValue;

                // Compute minute/second from timeOfServiceDesiredInPRS (epoch seconds)
                if (entry->timeOfServiceDesiredInPRS > 0) {
                    auto tsdEpoch = static_cast<time_t>(entry->timeOfServiceDesiredInPRS);
                    struct tm tsdUtc;
                    gmtime_r(&tsdEpoch, &tsdUtc);
                    auto minutePtr = (MinuteOfTheYear_t *)calloc(1, sizeof(MinuteOfTheYear_t));
                    *minutePtr = static_cast<MinuteOfTheYear_t>(tsdUtc.tm_yday * 24 * 60 + tsdUtc.tm_hour * 60 + tsdUtc.tm_min);
                    pkg->minute = minutePtr;

                    auto secondPtr = (DSecond_t *)calloc(1, sizeof(DSecond_t));
                    *secondPtr = static_cast<DSecond_t>(tsdUtc.tm_sec * 1000);
                    pkg->second = secondPtr;
                }

                // Compute duration from TED to TSD
                if (entry->timeOfEstimatedDepartureInPRS > entry->timeOfServiceDesiredInPRS) {
                    auto durationPtr = (DSecond_t *)calloc(1, sizeof(DSecond_t));
                    *durationPtr = static_cast<DSecond_t>(
                        (entry->timeOfEstimatedDepartureInPRS - entry->timeOfServiceDesiredInPRS) * 1000);
                    pkg->duration = durationPtr;
                }

                pkg->status = MapNTCIPstatusToSSM(entry->statusInPRS);
                asn_sequence_add(&signalStatus->sigStatus.list, pkg);
            }

            asn_sequence_add(&ssmPtr->status.list, signalStatus);
        }

        try {
            SsmEncodedMessage encodedSSM;
            MessageFrameMessage frame(ssmPtr);
            encodedSSM.set_data(TmxJ2735EncodedMessage<SignalStatusMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame));
            free(frame.get_j2735_data().get()); // NOSONAR: ASN.1 C struct allocated via calloc
            ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());

            encodedSSM.set_flags(IvpMsgFlags_RouteDSRC);
            encodedSSM.addDsrcMetadata(0xE0000015);
            BroadcastMessage(static_cast<tmx::routeable_message &>(encodedSSM));
            PLOG(logDEBUG) << "SSM broadcast from PRS table.";
        } catch (const std::invalid_argument &ex) {
            ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());
            PLOG(logERROR) << "Failed to encode/broadcast SSM: " << ex.what();
        }
    }

    void PriorityPlugin::BuildSSM(const RequestorState &state)
    {
        if (state.requests.empty()) {
            PLOG(logWARNING) << "No signal requests in state, skipping SSM build.";
            return;
        }

        auto ssmPtr = std::make_shared<SignalStatusMessage_t>();
        memset(ssmPtr.get(), 0, sizeof(SignalStatusMessage_t));

        struct timespec nowTs;
        clock_gettime(CLOCK_REALTIME, &nowTs);
        struct tm utcNow;
        gmtime_r(&nowTs.tv_sec, &utcNow);
        time_t nowEpoch = nowTs.tv_sec;

        auto timeStamp = (MinuteOfTheYear_t *)calloc(1, sizeof(MinuteOfTheYear_t));
        *timeStamp = static_cast<MinuteOfTheYear_t>(utcNow.tm_yday * 24 * 60 + utcNow.tm_hour * 60 + utcNow.tm_min);
        ssmPtr->timeStamp = timeStamp;
        ssmPtr->second = static_cast<DSecond_t>(utcNow.tm_sec * 1000 + nowTs.tv_nsec / 1000000);

        // ssmPtr->sequenceNumber increments for every new SSM broadcast
        _ssmSequenceCounter++;
        auto msgSequenceNumber = (Common_MsgCount_t *)calloc(1, sizeof(Common_MsgCount_t));
        *msgSequenceNumber = _ssmSequenceCounter;
        ssmPtr->sequenceNumber = msgSequenceNumber;

        std::map<long, std::vector<const SignalRequest *>> byIntersection;
        for (const auto &req : state.requests) {
            byIntersection[req.intersectionID].push_back(&req);
        }

        for (const auto &mapEntry : byIntersection) {
            long intID = mapEntry.first;
            const auto &reqs = mapEntry.second;

            // Create a key to track the package contents for this intersection
            // and detect changes and increment signalStatus->sequenceNumber accordingly
            std::ostringstream seqKey;
            for (const auto *req : reqs) {
                seqKey << static_cast<int>(req->requestID) << ","
                   << (req->rejected ? 1 : 0) << ","
                   << static_cast<int>(state.sequenceNumber) << ","
                   << req->timeOfService << ","
                   << req->timeOfDepart << ","
                   << static_cast<int>(req->inboundPresent) << ","
                   << req->inboundValue << ";";
            }
            std::string currentSeqKey = seqKey.str();

            auto lastSeqKeyIter = _lastSignalStatusKey.find(intID);
            if (lastSeqKeyIter == _lastSignalStatusKey.end() || lastSeqKeyIter->second != currentSeqKey) {
                _signalStatusSeqByIntersection[intID]++;
                _lastSignalStatusKey[intID] = currentSeqKey;
            }

            SignalStatus *signalStatus = (SignalStatus *)calloc(1, sizeof(SignalStatus));
            signalStatus->id.id = intID;
            signalStatus->sequenceNumber = _signalStatusSeqByIntersection[intID];

            for (const auto *req : reqs) {
                SignalStatusPackage *pkg = (SignalStatusPackage *)calloc(1, sizeof(SignalStatusPackage));
                pkg->requester = (SignalRequesterInfo *)calloc(1, sizeof(SignalRequesterInfo));
                pkg->requester->request = req->requestID;
                // requester->sequenceNumber is the SRM sequence number being responded to
                pkg->requester->sequenceNumber = state.sequenceNumber;

                if (state.vehicleID.size() == sizeof(StationID_t)) {
                    StationID_t stationID = 0;
                    std::memcpy(&stationID, state.vehicleID.data(), sizeof(StationID_t));
                    pkg->requester->id.choice.stationID = stationID;
                    pkg->requester->id.present = VehicleID_PR_stationID;
                }
                else {
                    pkg->requester->id.choice.entityID.buf =
                        (uint8_t *)calloc(state.vehicleID.size(), 1);
                    std::memcpy(pkg->requester->id.choice.entityID.buf,
                                state.vehicleID.data(), state.vehicleID.size());
                    pkg->requester->id.choice.entityID.size = state.vehicleID.size();
                    pkg->requester->id.present = VehicleID_PR_entityID;
                }

                auto rolePtr = (BasicVehicleRole_t *)calloc(1, sizeof(BasicVehicleRole_t));
                *rolePtr = static_cast<BasicVehicleRole_t>(state.role);
                pkg->requester->role = rolePtr;

                pkg->inboundOn.present = static_cast<IntersectionAccessPoint_PR>(req->inboundPresent);
                if (req->inboundPresent == IntersectionAccessPoint_PR_lane)
                    pkg->inboundOn.choice.lane = req->inboundValue;
                else if (req->inboundPresent == IntersectionAccessPoint_PR_approach)
                    pkg->inboundOn.choice.approach = req->inboundValue;

                if (req->etaMinute > 0) {
                    auto minutePtr = (MinuteOfTheYear_t *)calloc(1, sizeof(MinuteOfTheYear_t));
                    *minutePtr = static_cast<MinuteOfTheYear_t>(req->etaMinute);
                    pkg->minute = minutePtr;
                }
                else {
                    auto fallbackEta = nowEpoch + static_cast<time_t>(_estimatedArrivalTime);
                    struct tm fallbackUtc;
                    gmtime_r(&fallbackEta, &fallbackUtc);
                    auto minutePtr = (MinuteOfTheYear_t *)calloc(1, sizeof(MinuteOfTheYear_t));
                    *minutePtr = static_cast<MinuteOfTheYear_t>(fallbackUtc.tm_yday * 24 * 60 + fallbackUtc.tm_hour * 60 + fallbackUtc.tm_min);
                    pkg->minute = minutePtr;
                }

                if (req->etaSecond > 0) {
                    auto secondPtr = (DSecond_t *)calloc(1, sizeof(DSecond_t));
                    *secondPtr = static_cast<DSecond_t>(req->etaSecond);
                    pkg->second = secondPtr;
                }
                else {
                    auto fallbackEta = nowEpoch + static_cast<time_t>(_estimatedArrivalTime);
                    struct tm fallbackUtc;
                    gmtime_r(&fallbackEta, &fallbackUtc);
                    auto secondPtr = (DSecond_t *)calloc(1, sizeof(DSecond_t));
                    *secondPtr = static_cast<DSecond_t>(fallbackUtc.tm_sec * 1000);
                    pkg->second = secondPtr;
                }

                if (req->duration > 0) {
                    auto durationPtr = (DSecond_t *)calloc(1, sizeof(DSecond_t));
                    *durationPtr = static_cast<DSecond_t>(req->duration);
                    pkg->duration = durationPtr;
                }
                else {
                    auto durationPtr = (DSecond_t *)calloc(1, sizeof(DSecond_t));
                    *durationPtr = static_cast<DSecond_t>(
                        (_estimatedDepartureTime > _estimatedArrivalTime
                            ? _estimatedDepartureTime - _estimatedArrivalTime
                            : _estimatedDepartureTime) * 1000);
                    pkg->duration = durationPtr;
                }

                if (req->rejected) {
                    pkg->status = PrioritizationResponseStatus_rejected;
                }
                else if (req->requestType == PriorityRequestType_priorityCancellation) {
                    pkg->status = PrioritizationResponseStatus_watchOtherTraffic;
                }
                else {
                    pkg->status = PrioritizationResponseStatus_processing;
                }
                asn_sequence_add(&signalStatus->sigStatus.list, pkg);
            }
            asn_sequence_add(&ssmPtr->status.list, signalStatus);
        }

        try {
            SsmEncodedMessage encodedSSM;
            MessageFrameMessage frame(ssmPtr);
            encodedSSM.set_data(
                TmxJ2735EncodedMessage<SignalStatusMessage>::encode_j2735_message<
                    codec::uper<MessageFrameMessage>>(frame));
            free(frame.get_j2735_data().get()); // NOSONAR: ASN.1 C struct allocated via calloc
            ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());

            encodedSSM.set_flags(IvpMsgFlags_RouteDSRC);
            encodedSSM.addDsrcMetadata(0xE0000015);
            BroadcastMessage(static_cast<tmx::routeable_message &>(encodedSSM));
            PLOG(logINFO) << "SSM (processing) broadcast for " << state.requests.size() << " request(s).";
        } catch (const std::invalid_argument &ex) {
            ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());
            PLOG(logERROR) << "Failed to encode/broadcast SSM: " << ex.what();
        }
    }

} /* namespace PriorityPlugin */
