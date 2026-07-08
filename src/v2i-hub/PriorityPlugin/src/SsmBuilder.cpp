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

#include "SsmBuilder.hpp"

#include <cstdlib>
#include <cstring>
#include <map>
#include <sstream>
#include <vector>

#include <PluginLog.h>

using namespace tmx::messages;
using namespace tmx::utils;

namespace PriorityPlugin {

    namespace {
        // Note for allocation helpers below: 
        // ASN.1 allocations must come from the malloc family.
        // j2735_destroy releases the SSM tree with ASN_STRUCT_FREE, which calls free() on every node.
        // calloc's zeroing also marks all optional fields absent.

        // Allocates a zeroed ASN.1 struct.
        template <typename T>
        T *AllocAsn() {
            return static_cast<T *>(calloc(1, sizeof(T))); // NOSONAR - freed via free() by ASN_STRUCT_FREE
        }

        // Allocates a zeroed ASN.1 scalar.
        template <typename T>
        T *AllocAsn(T value) {
            auto *p = AllocAsn<T>();
            *p = value;
            return p;
        }

        // Allocates a zeroed byte buffer for an ASN.1 OCTET STRING.
        uint8_t *AllocAsnBuffer(size_t size) {
            return static_cast<uint8_t *>(calloc(size, 1)); // NOSONAR - freed via free() by ASN_STRUCT_FREE
        }

        // Allocates a SignalStatusMessage_t, owned by a shared_ptr.
        std::shared_ptr<SignalStatusMessage_t> MakeOwnedSsm() {
            return {AllocAsn<SignalStatusMessage_t>(),
                    [](SignalStatusMessage_t *p) { j2735::j2735_destroy<SsmTraits>(p); }};
        }

        // Populates the SSM header timeStamp and second fields from UTC time.
        void SetSsmTimestamp(SignalStatusMessage_t *ssm, const struct tm &utc, uint32_t subsecMs) {
            ssm->timeStamp = AllocAsn<MinuteOfTheYear_t>(utc.tm_yday * 24 * 60 + utc.tm_hour * 60 + utc.tm_min);
            ssm->second = static_cast<DSecond_t>(utc.tm_sec * 1000 + subsecMs);
        }

        // Sets the requester id (stationID or entityID) from raw vehicle ID bytes.
        void SetRequesterVehicleID(SignalRequesterInfo *requester, const std::vector<uint8_t> &vehicleID) {
            if (vehicleID.size() == sizeof(StationID_t)) {
                StationID_t stationID = 0;
                std::memcpy(&stationID, vehicleID.data(), sizeof(StationID_t));
                requester->id.choice.stationID = stationID;
                requester->id.present = VehicleID_PR_stationID;
            }
            else if (!vehicleID.empty()) {
                requester->id.choice.entityID.buf = AllocAsnBuffer(vehicleID.size());
                std::memcpy(requester->id.choice.entityID.buf, vehicleID.data(), vehicleID.size());
                requester->id.choice.entityID.size = vehicleID.size();
                requester->id.present = VehicleID_PR_entityID;
            }
        }

        // Allocates a status package with its requester populated from common SRM fields.
        SignalStatusPackage *MakeStatusPackage(uint8_t requestID, uint8_t srmSequenceNumber, const std::vector<uint8_t> &vehicleID, long role) {
            auto *pkg = AllocAsn<SignalStatusPackage>();
            pkg->requester = AllocAsn<SignalRequesterInfo>();
            pkg->requester->request = requestID;
            // requester->sequenceNumber is the SRM sequence number being responded to
            pkg->requester->sequenceNumber = srmSequenceNumber;
            SetRequesterVehicleID(pkg->requester, vehicleID);
            pkg->requester->role = AllocAsn<BasicVehicleRole_t>(role);
            return pkg;
        }

        // Effective status is the CO state when active, otherwise the PRS state.
        RequestStatus EffectiveStatus(const PriorityRequestEntry &e) {
            return IsActiveX(e.statusInCO) ? e.statusInCO : e.statusInPRS;
        }
    } // namespace

    long MapNTCIPstatusToSSM(RequestStatus status)
    {
        switch (status) {
            // PRS validated the request; the CO has not activated it yet.
            case RequestStatus::readyQueued:
                return PrioritizationResponseStatus_requested;

            // Still in the queue behind a prior request (readyOverridden can
            // return to readyQueued) or in a transient override negotiation.
            case RequestStatus::readyOverridden:
            case RequestStatus::activeOverride:
                return PrioritizationResponseStatus_processing;

            // Priority intervention is active, needs no timing adjustment, or has been completed
            case RequestStatus::activeProcessing:
            case RequestStatus::activeAdjustNotNeeded:
            case RequestStatus::activeNotOverridden:
            case RequestStatus::closedCompleted:
                return PrioritizationResponseStatus_granted;

            // Cancel in flight or cancelled by requester.
            // No more permission; watch for other traffic.
            case RequestStatus::activeCancel:
            case RequestStatus::closedCanceled:
                return PrioritizationResponseStatus_watchOtherTraffic;

            // CO refusals.
            case RequestStatus::closedTimerError:
            case RequestStatus::closedStrategyError:
            case RequestStatus::closedFlash:
                return PrioritizationResponseStatus_rejected;

            case RequestStatus::closedTimeToLiveError:
                return PrioritizationResponseStatus_maxPresence;
            case RequestStatus::reserviceError:
                return PrioritizationResponseStatus_reserviceLocked;
            default:
                return PrioritizationResponseStatus_unknown;
        }
    }

    std::shared_ptr<SignalStatusMessage_t> BuildSsmFromTable(std::array<PriorityRequestEntry, MAX_SERVICE_REQUESTS> &table, uint8_t maxBroadcastsPerStatus, time_t nowEpoch, SsmBroadcastState &ssmState)
    {
        std::map<long, std::vector<PriorityRequestEntry *>> byIntersection;
        for (auto &entry : table) {
            if (entry.statusInPRS == RequestStatus::idleNotValid) continue;
            if (auto effStatus = EffectiveStatus(entry);
                effStatus != entry.ssmLastStatus) {
                entry.ssmBroadcastCount = 0;
                entry.ssmLastStatus = effStatus;
            }
            if (entry.ssmBroadcastCount >= maxBroadcastsPerStatus) continue;
            entry.ssmBroadcastCount++;
            byIntersection[entry.intersectionID].push_back(&entry);
        }

        if (byIntersection.empty()) return nullptr;

        auto ssmPtr = MakeOwnedSsm();

        struct tm utcNow;
        gmtime_r(&nowEpoch, &utcNow);
        SetSsmTimestamp(ssmPtr.get(), utcNow, 0);

        // ssm sequenceNumber increments for every new SSM broadcast
        ssmState.ssmSequenceCounter++;
        ssmPtr->sequenceNumber = AllocAsn<Common_MsgCount_t>(ssmState.ssmSequenceCounter);

        for (const auto &[intID, entries] : byIntersection) {
            // Create a key to track the package contents for this intersection
            // and detect changes and increment signalStatus->sequenceNumber accordingly
            std::ostringstream seqKey;
            for (const auto *entry : entries) {
                seqKey << static_cast<int>(entry->requestID) << ","
                   << static_cast<int>(EffectiveStatus(*entry)) << ","
                   << static_cast<int>(entry->sequenceNumber) << ","
                   << entry->timeOfServiceDesiredInPRS << ","
                   << entry->timeOfEstimatedDepartureInPRS << ","
                   << static_cast<int>(entry->inboundPresent) << ","
                   << entry->inboundValue << ";";
            }
            std::string currentSeqKey = seqKey.str();

            if (auto lastSeqKeyIter = ssmState.lastSignalStatusKey.find(intID);
                lastSeqKeyIter == ssmState.lastSignalStatusKey.end() || lastSeqKeyIter->second != currentSeqKey) {
                ssmState.signalStatusSeqByIntersection[intID]++;
                ssmState.lastSignalStatusKey[intID] = currentSeqKey;
            }

            auto *signalStatus = AllocAsn<SignalStatus>();
            signalStatus->id.id = intID;
            signalStatus->sequenceNumber = ssmState.signalStatusSeqByIntersection[intID];

            for (const auto *entry : entries) {
                auto *pkg = MakeStatusPackage(entry->requestID, entry->sequenceNumber, entry->vehicleID, entry->role);

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
                    pkg->minute = AllocAsn<MinuteOfTheYear_t>(tsdUtc.tm_yday * 24 * 60 + tsdUtc.tm_hour * 60 + tsdUtc.tm_min);
                    pkg->second = AllocAsn<DSecond_t>(tsdUtc.tm_sec * 1000);
                }

                // Compute duration from TED to TSD
                if (entry->timeOfEstimatedDepartureInPRS > entry->timeOfServiceDesiredInPRS) {
                    pkg->duration = AllocAsn<DSecond_t>(
                        (entry->timeOfEstimatedDepartureInPRS - entry->timeOfServiceDesiredInPRS) * 1000);
                }

                pkg->status = MapNTCIPstatusToSSM(EffectiveStatus(*entry));
                asn_sequence_add(&signalStatus->sigStatus.list, pkg);
            }

            asn_sequence_add(&ssmPtr->status.list, signalStatus);
        }

        return ssmPtr;
    }

    std::shared_ptr<SignalStatusMessage_t> BuildSsmFromRequestor(const RequestorState &state, uint64_t nowMs, uint16_t estimatedArrivalTime, uint16_t estimatedDepartureTime, SsmBroadcastState &ssmState)
    {
        if (state.requests.empty()) {
            PLOG(logWARNING) << "No signal requests in state, skipping SSM build.";
            return nullptr;
        }

        auto ssmPtr = MakeOwnedSsm();

        auto nowEpoch = static_cast<time_t>(nowMs / 1000);
        auto subsecMs = static_cast<uint32_t>(nowMs % 1000);
        struct tm utcNow;
        gmtime_r(&nowEpoch, &utcNow);
        SetSsmTimestamp(ssmPtr.get(), utcNow, subsecMs);

        // ssm sequenceNumber increments for every new SSM broadcast
        ssmState.ssmSequenceCounter++;
        ssmPtr->sequenceNumber = AllocAsn<Common_MsgCount_t>(ssmState.ssmSequenceCounter);

        std::map<long, std::vector<const SignalRequest *>> byIntersection;
        for (const auto &req : state.requests) {
            byIntersection[req.intersectionID].push_back(&req);
        }

        for (const auto &[intID, reqs] : byIntersection) {
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

            if (auto lastSeqKeyIter = ssmState.lastSignalStatusKey.find(intID);
                lastSeqKeyIter == ssmState.lastSignalStatusKey.end() || lastSeqKeyIter->second != currentSeqKey) {
                ssmState.signalStatusSeqByIntersection[intID]++;
                ssmState.lastSignalStatusKey[intID] = currentSeqKey;
            }

            auto *signalStatus = AllocAsn<SignalStatus>();
            signalStatus->id.id = intID;
            signalStatus->sequenceNumber = ssmState.signalStatusSeqByIntersection[intID];

            for (const auto *req : reqs) {
                auto *pkg = MakeStatusPackage(req->requestID, state.sequenceNumber, state.vehicleID, state.role);

                pkg->inboundOn.present = static_cast<IntersectionAccessPoint_PR>(req->inboundPresent);
                if (req->inboundPresent == IntersectionAccessPoint_PR_lane)
                    pkg->inboundOn.choice.lane = req->inboundValue;
                else if (req->inboundPresent == IntersectionAccessPoint_PR_approach)
                    pkg->inboundOn.choice.approach = req->inboundValue;

                if (req->etaMinute > 0) {
                    pkg->minute = AllocAsn<MinuteOfTheYear_t>(req->etaMinute);
                }
                else {
                    auto fallbackEta = nowEpoch + static_cast<time_t>(estimatedArrivalTime);
                    struct tm fallbackUtc;
                    gmtime_r(&fallbackEta, &fallbackUtc);
                    pkg->minute = AllocAsn<MinuteOfTheYear_t>(fallbackUtc.tm_yday * 24 * 60 + fallbackUtc.tm_hour * 60 + fallbackUtc.tm_min);
                }

                if (req->etaSecond > 0) {
                    pkg->second = AllocAsn<DSecond_t>(req->etaSecond);
                }
                else {
                    auto fallbackEta = nowEpoch + static_cast<time_t>(estimatedArrivalTime);
                    struct tm fallbackUtc;
                    gmtime_r(&fallbackEta, &fallbackUtc);
                    pkg->second = AllocAsn<DSecond_t>(fallbackUtc.tm_sec * 1000);
                }

                if (req->duration > 0) {
                    pkg->duration = AllocAsn<DSecond_t>(req->duration);
                }
                else {
                    pkg->duration = AllocAsn<DSecond_t>(
                        (estimatedDepartureTime > estimatedArrivalTime
                            ? estimatedDepartureTime - estimatedArrivalTime
                            : estimatedDepartureTime) * 1000);
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

        return ssmPtr;
    }

} /* namespace PriorityPlugin */
