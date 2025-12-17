/**
 * Copyright (C) 2024 LEIDOS.
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

#include <UdpServer.h>
#include <SNMPClient.h>
#include <tmx/j2735_messages/SpatMessage.hpp>
#include <PluginLog.h>
#include <tmx/messages/byte_stream.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <tmx/TmxException.hpp>
#include <gtest/gtest_prod.h>  
#include <map>
#include <bitset>
#include <carma-clock/carma_clock.h>


#include "NTCIP1202.h"
#include "NTCIP1202OIDs.h"



namespace SpatPlugin {
    /**
     * @brief Class to represent Traffic Signal Controller (TSC or SC) connection. Includes a UDP Server for listening for 
     * SPaT data broadcast from TSC. Also includes an SNMP Client for request or setting status of SNMP objects described
     * in NTCIP 1202 (Object Definitions for Actuated Signal Controllers).
     */
    class SignalControllerConnection
    {
        private:
            /**
             * @brief UDP Server for receiving SPaT packets from TSC.
             */
            std::shared_ptr<tmx::utils::UdpServer> spatPacketReceiver;
            /**
             * @brief SNMP Client for requesting or setting status of SNMP Objects on 
             * TSC.
             */
            std::shared_ptr<tmx::utils::snmp_client> scSNMPClient;
            /**
             * @brief String that describes phase to signal group mapping configured on 
             * the TSC. TODO: Remove in place of SNMP requests on tables.
             */
            std::string signalGroupMapping;
            /**
             * @brief String name of intersection in SPaT messages.
             */
            std::string intersectionName;
            /**
             * @brief Numeric identifier for intersection in SPaT messages.
             */
            unsigned int intersectionId;

            const static unsigned int SPAT_BINARY_BUFFER_SIZE = 1000;

            const static unsigned int UDP_SERVER_TIMEOUT_MS = 1000;


            std::map<std::string, bool> intersectionStatus = {
                {"Manual Control Is Enabled", false},
                {"Stop Time Is Activated", false},
                {"Failure Flash", false},
                {"Preemption Is Active", false},
                {"Signal Priority Is Active", false},
                {"Fixed Time Operation", false},
                {"Traffic Dependent Operation", false},
                {"Standby Operation", false},
                {"Failure Mode", false},
                {"Off", false},
                {"Recent MAP Message Update", false},
                {"Recent Change in MAP Assigned Lane Ids Used", false},
                {"No Valid MAP No Available At This Time", false},
                {"No Valid SPAT is Available At This Time", false},
            };
            friend class TestSignalControllerConnection;

        public:
            /**
             * @brief Constructor for Signal Controller Connection.
             * @param localIp IP address of device connecting to signal controller. This will be the IP of the UDP Server listening for SPaT data.
             * @param localPort port on which to listen for SPaT data.
             * @param signalGroupMapping JSON mapping of phases to signal groups
             * @param scIp IP address of TSC
             * @param scSNMPPort port of SNMP Server on TSC
             * @param intersectionName Name of intersection
             * @param intersectionID Intersection ID.
             */
            SignalControllerConnection(const std::string &localIp, unsigned int localPort, const std::string &signalGroupMapping, const std::string &scIp, unsigned int scSNMPPort, const std::string &scSNMPCommunity, const std::string &intersectionName, unsigned int intersectionID);
           
            /**
             * @brief Method attempts to send SNMP SET requests to initialize the TSC. NOTE: Some of the
             * OIDs in called in this initialize method may not be exposed by a TSC depending on which
             * version of the NTCIP 1202 standard they are complaint with. To avoid failures please use
             * the bool flags to indicate which OIDs need to be set for initialization.
             * @param enable_spat bool flag on whether to attempt to set enable spat to true (NOT available for NTCIP 1202 versions >= 3 )
             * @return true if successful and false if not.
             */
            bool initializeSignalControllerConnection(bool enable_spat) const;
            /**
             * @brief Method to receive SPaT data in binary format from TSC.
             * @param spat an empty SPaT pointer to which the SPAT data will be written.
             * @param timeMs current time in ms from epoch to use for message timestamp.
             */
            void receiveBinarySPAT( SPAT * const spat, const std::shared_ptr<fwha_stol::lib::time::CarmaClock> &clock);
            /**
             * @brief Method to receive SPaT data in UPER Hex format from TSC.
             * @param spatEncoded_ptr Empty SpatEncodedMessage to which the UPER encoded SPaT data will be written.
             */
            void receiveUPERSPAT(std::shared_ptr<tmx::messages::SpatEncodedMessage> &spatEncoded_ptr);
            /**
             * @brief Static function to calculate interval between two spat messages given the ms epoch timestamps
             * of each.
             * @param lastSpatMessage epoch ms timestamp of the last spat message
             * @param currentSpatMessage epoch ms of the current receved spat message. Should be larger than lastSpatMessage
             * @returns interval in ms
             * @throws TmxException if interval exceeds maximum allowable value from CTI 4501 of 300 ms
             */
            static uint calculateSPaTInterval(uint64_t lastSpatMessage, uint64_t currentSpatMessage);

            void updateIntersectionStatus(const IntersectionStatusObject_t &status);

            std::map<std::string, bool> getIntersectionStatus() const;

            const static unsigned int SPAT_INTERVAL_MAX_THRESHOLD_MS = 300;

    };

}