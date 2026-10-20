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

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace PriorityPlugin {

    // Size of the OER-encoded service request OCTET STRING (10 rows × 10 bytes + prsBusy(1) + 9 reserved = 110)
    static constexpr size_t SERVICE_REQUEST_SIZE = 110;

    // Number of rows in the prsServiceRequest and priorityStrategyRequestedTable
    static constexpr size_t MAX_SERVICE_REQUESTS = 10;

    // Bytes per row in prsServiceRequest. strategy(1) + TSD(4) + TED(4) + status(1) = 10
    static constexpr size_t SERVICE_REQUEST_ROW_SIZE = 10;

    // Offset of the prsBusy and coBusy byte in the 110-byte prsServiceRequest
    static constexpr size_t SERVICE_REQUEST_BUSY_OFFSET = MAX_SERVICE_REQUESTS * SERVICE_REQUEST_ROW_SIZE; // byte 100

    // Size of the OER-encoded priority request OCTET STRING (NTCIP 1211 PRS-MIB1 5.1.2.8)
    static constexpr size_t PRIORITY_REQUEST_SIZE = 29;

    // Size of the OER-encoded priority cancel/clear OCTET STRING (NTCIP 1211 PRS-MIB1 5.1.2.5/5.1.2.6)
    static constexpr size_t PRIORITY_CANCEL_SIZE = 21;

    // Vehicle ID field size within the NTCIP 1211 priority request
    static constexpr size_t VEHICLE_ID_FIELD_SIZE = 17;

    /**
     * @brief NTCIP 1211 priorityRequestStatusInPRS and priorityStrategyRequestStatusInCO state values, per CO-MIB1 5.2.1.2.5 and PRS-MIB1 5.1.1.1.9.
     */
    enum class RequestStatus : uint8_t {
        idleNotValid          = 1,
        readyQueued           = 2,
        readyOverridden       = 3,
        activeProcessing      = 4,
        activeCancel          = 5,
        activeOverride        = 6,
        activeNotOverridden   = 7,
        closedCanceled        = 8,
        reserviceError        = 9,
        closedTimeToLiveError = 10,
        closedTimerError      = 11,
        closedStrategyError   = 12,
        closedCompleted       = 13,
        activeAdjustNotNeeded = 14,
        closedFlash           = 15
    };

    // Classify a RequestStatus by its name prefix, independent of numeric enum value.
    constexpr bool IsReadyX(RequestStatus s) {
        return s == RequestStatus::readyQueued ||
               s == RequestStatus::readyOverridden;
    }
    constexpr bool IsActiveX(RequestStatus s) {
        return s == RequestStatus::activeProcessing ||
               s == RequestStatus::activeCancel ||
               s == RequestStatus::activeOverride ||
               s == RequestStatus::activeNotOverridden ||
               s == RequestStatus::activeAdjustNotNeeded;
    }
    constexpr bool IsClosedX(RequestStatus s) {
        return s == RequestStatus::closedCanceled ||
               s == RequestStatus::closedTimeToLiveError ||
               s == RequestStatus::closedTimerError ||
               s == RequestStatus::closedStrategyError ||
               s == RequestStatus::closedCompleted ||
               s == RequestStatus::closedFlash;
    }

    /**
     * @brief One row of the PRS priorityRequestTable (NTCIP 1211 5.1.1.1). The PRS maintains up to MAX_SERVICE_REQUESTS of these.
     */
    struct PriorityRequestEntry {
        // Fields populated using incoming SRM
        uint8_t  requestID              = 0;
        std::vector<uint8_t> vehicleID;
        uint8_t  vehicleClassType       = 10;  // other
        uint8_t  vehicleClassLevel      = 1;   // highest priority level
        uint8_t  serviceStrategyNumber  = 0;   // no value, placeholder (1-255 valid)

        // Times expressed as global time (epoch seconds)
        uint32_t timeOfServiceDesiredInPRS       = 0;
        uint32_t timeOfEstimatedDepartureInPRS   = 0;

        // PRS internal bookkeeping
        uint32_t timeOfMessage          = 0;   // epoch seconds when PRS received the request
        uint32_t timeToLive             = 0;   // epoch seconds at which the request expires

        // PRS-owned state per NTCIP 1211 5.1.1.1.9.
        RequestStatus statusInPRS       = RequestStatus::idleNotValid;

        // CO's most recently reported state per NTCIP 1211 5.2.1.2.5. Tracked separately 
        // so the PRS does not echo CO-owned activeX values back on subsequent SETs.
        RequestStatus statusInCO        = RequestStatus::idleNotValid;

        // Intersection ID that each request targets
        long     intersectionID         = 0;

        // SRM sequence number (for SSM building)
        uint8_t  sequenceNumber         = 0;

        // Original BasicVehicleRole from the SRM requestor (for SSM requester->role)
        long     role                   = 0;

        // SSM broadcast limiter: count resets on status transition
        uint8_t  ssmBroadcastCount      = 0;
        RequestStatus ssmLastStatus     = RequestStatus::idleNotValid;

        // Inbound lane/approach from the SRM (for SSM inboundOn)
        uint8_t  inboundPresent         = 0;   // IntersectionAccessPoint_PR value
        long     inboundValue           = 0;
    };

    /**
     * @brief One row of the CO response within a prsServiceRequest GET response. These are the CO-side equivalents returned when PRS GETs prsServiceRequest.
     */
    struct CoServiceResponseRow {
        uint8_t  strategyRequested                       = 0;
        uint32_t requestedTimeOfServiceDesired           = 0;
        uint32_t requestedTimeOfEstimatedDeparture       = 0;
        RequestStatus requestStatusInCO                  = RequestStatus::idleNotValid;
    };

    /**
     * @brief State of a priority request the PRG has sent to a PRS/CO.
     *        Used to distinguish active requests from canceled ones awaiting a clear.
     */
    enum class PrgRequestState : uint8_t {
        sent,
        canceled
    };

    /**
     * @brief Priority request the PRG has sent, keyed by requestID, intersectionID, and vehicleID. 
     *        Drives update vs. new requests and the post-cancel clear sweep (PRG mode).
     */
    struct PrgTrackedRequest {
        uint8_t requestID;
        long intersectionID;
        std::vector<uint8_t> vehicleID;
        uint8_t classType;
        uint8_t classLevel;
        uint8_t strategyNumber;
        uint64_t sentTimeMs;
        PrgRequestState state = PrgRequestState::sent;
    };

    /**
     * @brief Signal request package decoded from an SRM. Captures the timing and
     *        lane fields needed to build an SSM and (PRG mode) the outgoing priority request.
     */
    struct SignalRequest {
        uint8_t requestID;
        long intersectionID;
        long requestType;
        uint16_t timeOfService;
        uint16_t timeOfDepart;
        bool rejected = false;
        uint8_t inboundPresent = 0;
        long inboundValue = 0;
        long etaMinute = 0;
        long etaSecond = 0;
        long duration = 0;
    };

    /**
     * @brief All requests from a unique requestor, keyed by vehicleID.
     *        Built during SRM dispatch and consumed when building the responding SSM (PRG mode).
     */
    struct RequestorState {
        std::vector<uint8_t> vehicleID;
        uint8_t classType;
        uint8_t sequenceNumber;
        uint32_t timeOfRequest;
        std::vector<SignalRequest> requests;
        long role = 0;
    };

} /* namespace PriorityPlugin */
