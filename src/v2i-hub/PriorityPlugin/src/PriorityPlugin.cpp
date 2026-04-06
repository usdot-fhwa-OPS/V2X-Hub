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
    PriorityPlugin::PriorityPlugin(const std::string &name) : TmxMessageManager(name)
    {
        AddMessageFilter <SrmMessage> (this, &PriorityPlugin::HandleSRM);
        SubscribeToMessages();
    }

    PriorityPlugin::~PriorityPlugin()
    {
    }

    void PriorityPlugin::OnStateChange(IvpPluginState state)
    {
        TmxMessageManager::OnStateChange(state);
		if (state == IvpPluginState_registered) {
			UpdateConfigSettings();
		}
    }

    void PriorityPlugin::OnConfigChanged(const char *key, const char *value)
    {
        TmxMessageManager::OnConfigChanged(key, value);
		UpdateConfigSettings();
    }

    void PriorityPlugin::OnMessageReceived(tmx::routeable_message &msg)
    {
        PLOG(logDEBUG1) << "Routable Message: " << msg.to_string();
        if (PluginClient::IsJ2735Message(msg))
        {
        }
    }

    void PriorityPlugin::UpdateConfigSettings()
    {
        std::string tscControllersJson;
        GetConfigValue<std::string>("TSC_Controllers", tscControllersJson);
        GetConfigValue<std::string>("TSC_SNMP_Community", _snmpCommunity);
        
        GetConfigValue<uint8_t>("VehicleClassLevel", _classLevelStr);
        GetConfigValue<uint8_t>("ServiceStrategyNumber", _strategyStr);
        GetConfigValue<uint16_t>("TimeOfServiceDesired", _tsd);
        GetConfigValue<uint16_t>("TimeOfEstimatedDeparture", _ted);

        // Parse the TSC_Controllers JSON array and create an SNMP client per entry
        _controllers.clear();
        try {
            boost::property_tree::ptree pt;
            std::istringstream iss(tscControllersJson);
            boost::property_tree::read_json(iss, pt);

            for (const auto &entry : pt) {
                ControllerInfo info;
                info.intersectionID = entry.second.get<long>("IntersectionID");
                info.ip = entry.second.get<std::string>("IP");
                info.port = entry.second.get<uint16_t>("Port");

                try {
                    info.snmpClient = std::make_shared<snmp_client>(
                        info.ip, info.port, _snmpCommunity,
                        "", "", "",
                        SNMP_VERSION_1);
                } catch (const snmp_client_exception &e) {
                    PLOG(logERROR) << "Failed to create SNMP client for IntersectionID="
                                   << info.intersectionID << " (" << info.ip << ":" << info.port
                                   << "): " << e.what();
                    continue;
                }

                PLOG(logINFO) << "Configured controller: IntersectionID=" << info.intersectionID
                               << " IP=" << info.ip << " Port=" << info.port;
                _controllers[info.intersectionID] = std::move(info);
            }
        } catch (const boost::property_tree::json_parser_error &e) {
            PLOG(logERROR) << "Failed to parse TSC_Controllers JSON: " << e.what();
        } catch (const boost::property_tree::ptree_error &e) {
            PLOG(logERROR) << "Invalid TSC_Controllers entry: " << e.what();
        }

        PLOG(logINFO) << "PriorityPlugin configured: " << _controllers.size() << " controller(s)"
                                   << " Community=" << _snmpCommunity
                                   << " ClassLevel=" << _classLevelStr
                                   << " Strategy=" << _strategyStr
                                   << " TimeOfServiceDesired=" << _tsd
                                   << " TimeOfEstimatedDeparture=" << _ted;
    }

    void PriorityPlugin::HandleSRM(SrmMessage &msg, tmx::routeable_message &routeableMsg)
    {
        PLOG(logINFO) << "Received Signal Request Message (SRM)";

        auto srm = msg.get_j2735_data();
        if (!srm) {
            PLOG(logWARNING) << "SRM decode returned null, skipping.";
            _skippedMessages++;
            SetStatus(_keySkippedMessages, _skippedMessages);
            return;
        }

        // Validate that the SRM contains requests
        if (!srm->requests || srm->requests->list.count <= 0) {
            PLOG(logWARNING) << "SRM contains no signal request packages, skipping.";
            _skippedMessages++;
            SetStatus(_keySkippedMessages, _skippedMessages);
            return;
        }

        // Extract the vehicle ID from the requestor
        const uint8_t *vehicleIDBytes = nullptr;
        size_t vehicleIDLen = 0;
        if (srm->requestor.id.present == VehicleID_PR_entityID) {
            vehicleIDBytes = srm->requestor.id.choice.entityID.buf;
            vehicleIDLen = srm->requestor.id.choice.entityID.size;
        } else if (srm->requestor.id.present == VehicleID_PR_stationID) {
            // stationID is a 4-byte value; treat its memory as bytes
            vehicleIDBytes = reinterpret_cast<const uint8_t *>(&srm->requestor.id.choice.stationID);
            vehicleIDLen = sizeof(srm->requestor.id.choice.stationID);
        }

        if (!vehicleIDBytes || vehicleIDLen == 0) {
            PLOG(logWARNING) << "SRM has no identifiable vehicle ID, skipping.";
            _skippedMessages++;
            SetStatus(_keySkippedMessages, _skippedMessages);
            return;
        }

        // Build the map key from the raw vehicle ID bytes
        std::string vehicleKey(reinterpret_cast<const char *>(vehicleIDBytes), vehicleIDLen);

        // Determine vehicle class type from the requestor role
        long role = 0;
        if (srm->requestor.type != nullptr) {
            role = srm->requestor.type->role;
        }
        uint8_t classType = MapVehicleClassType(role);

        // Current epoch time for priorityRequestTimeOfRequest
        uint32_t timeOfRequest = static_cast<uint32_t>(std::time(nullptr));

        // Current time for computing relative offsets from "now"
        time_t nowEpoch = std::time(nullptr);
        struct tm utcNow;
        gmtime_r(&nowEpoch, &utcNow);
        // Compute current minute-of-year and second-within-minute
        auto currentDayOfYear = utcNow.tm_yday;
        auto currentMinuteOfYear = static_cast<long>(currentDayOfYear) * 1440L
                                 + static_cast<long>(utcNow.tm_hour) * 60L
                                 + static_cast<long>(utcNow.tm_min);
        auto currentMsInMinute = static_cast<long>(utcNow.tm_sec) * 1000L;

        // Build the requestor state, replacing any prior entry for this vehicle
        auto newSeq = srm->sequenceNumber ? static_cast<uint8_t>(*srm->sequenceNumber) : 0;
        auto existing = _requestorStates.find(vehicleKey);
        if (existing != _requestorStates.end() && existing->second.sequenceNumber == newSeq) {
            PLOG(logDEBUG1) << "SRM sequence number unchanged (" << static_cast<int>(newSeq)
                            << ") for this vehicle, discarding.";
            return;
        }

        RequestorState &state = _requestorStates[vehicleKey];
        state.vehicleID.assign(vehicleIDBytes, vehicleIDBytes + vehicleIDLen);
        state.classType = classType;
        state.sequenceNumber = newSeq;
        state.timeOfRequest = timeOfRequest;
        state.requests.clear();

        // Process each SignalRequestPackage in the SRM
        for (int i = 0; i < srm->requests->list.count; i++) {
            auto *pkg = srm->requests->list.array[i];
            if (!pkg) {
                continue;
            }

            auto requestID = static_cast<uint8_t>(pkg->request.requestID);
            auto requestType = pkg->request.requestType;
            long intersectionID = pkg->request.id.id;

            // Compute priorityRequestTimeOfServiceDesired:
            // NTCIP 1211 5.1.1.1.7 — relative seconds to arrive at the intersection
            // stopping point from receipt of the message.
            // SRM provides MinuteOfTheYear (absolute) and DSecond (ms within minute).
            // Convert the absolute ETA to a relative offset from "now".
            auto timeOfService = _tsd;  // default per configuration
            long etaOffsetMs = 0;
            if (pkg->minute) {
                auto etaMinuteOfYear = static_cast<long>(*pkg->minute);
                long etaMs = 0;
                if (pkg->second) {
                    etaMs = static_cast<long>(*pkg->second);
                }
                // Total ms from start-of-year for ETA
                auto etaTotalMs = etaMinuteOfYear * 60L * 1000L + etaMs;
                // Total ms from start-of-year for now
                auto nowTotalMs = currentMinuteOfYear * 60L * 1000L + currentMsInMinute;
                etaOffsetMs = etaTotalMs - nowTotalMs;
                // Handle year wrap-around
                if (etaOffsetMs < 0) {
                    etaOffsetMs += 525960L * 60L * 1000L;  // MinuteOfTheYear max ≈ 365.25 days
                }
                auto etaOffsetSec = etaOffsetMs / 1000L;
                timeOfService = static_cast<uint16_t>(
                    std::min(65535L, std::max(1L, etaOffsetSec)));
            }

            // Compute priorityRequestTimeOfEstimatedDeparture:
            // NTCIP 1211 5.1.1.1.8 — relative seconds of estimated departure
            // from the intersection from receipt of the message.
            auto timeOfDepart = _ted;  // default per configuration
            if (pkg->duration) {
                // Duration extends past the ETA
                auto departOffsetMs = etaOffsetMs + static_cast<long>(*pkg->duration);
                auto departOffsetSec = departOffsetMs / 1000L;
                timeOfDepart = static_cast<uint16_t>(
                    std::min(65535L, std::max(1L, departOffsetSec)));
            }

            // Record this package in the requestor state
            state.requests.push_back({requestID, intersectionID, requestType, timeOfService, timeOfDepart});

            // Encode the 29-byte NTCIP 1211 priority request
            std::vector<uint8_t> encoded = EncodePriorityRequest(
                requestID,
                vehicleIDBytes,
                vehicleIDLen,
                classType,
                _classLevelStr,
                _strategyStr,
                timeOfService,
                timeOfDepart,
                timeOfRequest);

            PLOG(logINFO) << "Sending NTCIP 1211 priority request for requestID="
                                       << static_cast<int>(requestID)
                                       << " intersectionID=" << intersectionID;

            auto it = _controllers.find(intersectionID);
            if (it == _controllers.end()) {
                PLOG(logWARNING) << "No controller configured for IntersectionID="
                                  << intersectionID << ", skipping request.";
                _skippedMessages++;
                SetStatus(_keySkippedMessages, _skippedMessages);
                continue;
            }

            const auto &controller = it->second;
            PLOG(logINFO) << "Routing to controller " << controller.ip << ":" << controller.port;

            bool success = SendRequest(controller.snmpClient, NTCIP1211_PRIORITY_REQUEST_ABSOLUTE_OID, encoded);
            if (success) {
                _priorityRequestsSent++;
                SetStatus(_keyPriorityRequestsSent, _priorityRequestsSent);
                PLOG(logINFO) << "Priority request sent successfully.";
            } else {
                PLOG(logERROR) << "Failed to send priority request via SNMP.";
            }
        }

        BuildSSM(state);
    }

    uint8_t PriorityPlugin::MapVehicleClassType(long role) const
    {
        switch (role) {
            case 5:  // roadRescue
            case 6:  // emergency
            case 7:  // safetyCar
            case 11: // roadSideSource
            case 12: // police
            case 13: // fire
            case 14: // ambulance 
                return 1;   // Highest priority — emergency vehicles
            case 1:  // publicTransport
            case 16: // transit
                return 3;   // Transit priority
            case 4:  // roadWork
            case 15: // dot
                return 5;   // Maintenance/supervisor
            case 3:  // dangerousGoods
            case 9:  // truck
            case 17: // slowMoving
            case 18: // stopNgo
                return 7;   // Commercial/freight
            default:
                return 10;  // Lowest priority
        }
    }

    std::vector<uint8_t> PriorityPlugin::EncodePriorityRequest(uint8_t requestID, const uint8_t *vehicleID, size_t vehicleIDLen, uint8_t classType, uint8_t classLevel, uint8_t strategyNum, uint16_t timeOfService, uint16_t timeOfDepart, uint32_t timeOfRequest) const
    {
        std::vector<uint8_t> buf(PRIORITY_REQUEST_SIZE, 0);

        // Byte 0: priorityRequestID (1 byte)
        buf[0] = requestID;

        // Bytes 1-17: priorityRequestVehicleID (17 bytes, zero-padded)
        if (vehicleID && vehicleIDLen > 0) 
        {
            size_t copyLen = std::min(vehicleIDLen, VEHICLE_ID_FIELD_SIZE);
            std::memcpy(&buf[1], vehicleID, copyLen);
        }

        // Byte 18: priorityRequestVehicleClassType (1 byte)
        buf[18] = classType;

        // Byte 19: priorityRequestVehicleClassLevel (1 byte)
        buf[19] = classLevel;

        // Byte 20: priorityRequestServiceStrategyNumber (1 byte)
        buf[20] = strategyNum;

        // Bytes 21-22: priorityRequestTimeOfServiceDesired (2 bytes, big-endian)
        buf[21] = static_cast<uint8_t>((timeOfService >> 8) & 0xFF);
        buf[22] = static_cast<uint8_t>(timeOfService & 0xFF);

        // Bytes 23-24: priorityRequestTimeOfEstimatedDeparture (2 bytes, big-endian)
        buf[23] = static_cast<uint8_t>((timeOfDepart >> 8) & 0xFF);
        buf[24] = static_cast<uint8_t>(timeOfDepart & 0xFF);

        // Bytes 25-28: priorityRequestTimeOfRequest (4 bytes, big-endian)
        buf[25] = static_cast<uint8_t>((timeOfRequest >> 24) & 0xFF);
        buf[26] = static_cast<uint8_t>((timeOfRequest >> 16) & 0xFF);
        buf[27] = static_cast<uint8_t>((timeOfRequest >> 8) & 0xFF);
        buf[28] = static_cast<uint8_t>(timeOfRequest & 0xFF);

        return buf;
    }

    bool PriorityPlugin::SendRequest(const std::shared_ptr<snmp_client> &client, const std::string &oidStr, const std::vector<uint8_t> &data)
    {
        if (!client) {
            PLOG(logERROR) << "SNMP client not initialized, cannot send priority request.";
            return false;
        }

        // Build the snmp_response_obj with the raw bytes as a STRING for SET
        snmp_response_obj val;
        val.type = snmp_response_obj::response_type::STRING;
        val.val_string.assign(data.begin(), data.end());

        bool success = client->process_snmp_request(
            oidStr, request_type::SET, val);

        if (success) {
            PLOG(logDEBUG) << "SNMP SET successful for OID: " << oidStr;
        } else {
            PLOG(logERROR) << "SNMP SET failed for OID: " << oidStr;
        }
        return success;
    }

    void PriorityPlugin::BuildSSM(const RequestorState &state)
    {
        if (state.requests.empty()) {
            PLOG(logWARNING) << "No signal requests in state, skipping SSM build.";
            return;
        }

        auto ssmPtr = std::make_shared<SignalStatusMessage_t>();
        memset(ssmPtr.get(), 0, sizeof(SignalStatusMessage_t));

        // Set second: milliseconds within the current UTC minute
        time_t nowEpoch = std::time(nullptr);
        struct tm utcNow;
        gmtime_r(&nowEpoch, &utcNow);
        ssmPtr->second = static_cast<DSecond_t>(utcNow.tm_sec * 1000);

        // Group signal requests by intersection ID; each intersection gets one SignalStatus entry
        std::map<long, std::vector<const SignalRequest *>> byIntersection;
        for (const auto &req : state.requests) {
            byIntersection[req.intersectionID].push_back(&req);
        }

        for (const auto &entry : byIntersection) {
            long intID = entry.first;
            const auto &reqs = entry.second;

            SignalStatus *signalStatus = (SignalStatus *)calloc(1, sizeof(SignalStatus));
            signalStatus->id.id = intID;
            signalStatus->sequenceNumber = state.sequenceNumber;

            for (const auto *req : reqs) {
                SignalStatusPackage *pkg = (SignalStatusPackage *)calloc(1, sizeof(SignalStatusPackage));
                pkg->requester = (SignalRequesterInfo *)calloc(1, sizeof(SignalRequesterInfo));
                pkg->requester->request = req->requestID;
                pkg->requester->sequenceNumber = state.sequenceNumber;

                // Reconstruct the vehicle ID from stored raw bytes
                if (state.vehicleID.size() == sizeof(StationID_t)) {
                    StationID_t sid = 0;
                    std::memcpy(&sid, state.vehicleID.data(), sizeof(StationID_t));
                    pkg->requester->id.choice.stationID = sid;
                    pkg->requester->id.present = VehicleID_PR_stationID;
                } else {
                    pkg->requester->id.choice.entityID.buf =
                        (uint8_t *)calloc(state.vehicleID.size(), 1);
                    std::memcpy(pkg->requester->id.choice.entityID.buf,
                                state.vehicleID.data(), state.vehicleID.size());
                    pkg->requester->id.choice.entityID.size = state.vehicleID.size();
                    pkg->requester->id.present = VehicleID_PR_entityID;
                }

                pkg->status = PrioritizationResponseStatus_processing;
                asn_sequence_add(&signalStatus->sigStatus.list, pkg);
            }

            asn_sequence_add(&ssmPtr->status.list, signalStatus);
        }

        // Encode and broadcast the SSM
        try {
            SsmEncodedMessage encodedSSM;
            MessageFrameMessage frame(ssmPtr);
            encodedSSM.set_data(
                TmxJ2735EncodedMessage<SignalStatusMessage>::encode_j2735_message<
                    codec::uper<MessageFrameMessage>>(frame));
            free(frame.get_j2735_data().get());
            ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());

            encodedSSM.set_flags(IvpMsgFlags_RouteDSRC);
            encodedSSM.addDsrcMetadata(0xE0000015);
            BroadcastMessage(static_cast<tmx::routeable_message &>(encodedSSM));
            PLOG(logINFO) << "SSM (processing) broadcast for " << state.requests.size() << " request(s).";
        } catch (const std::exception &ex) {
            ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());
            PLOG(logERROR) << "Failed to encode/broadcast SSM: " << ex.what();
        }
    }

} /* namespace PriorityPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<PriorityPlugin::PriorityPlugin>("Priority Plugin", argc, argv);
}
