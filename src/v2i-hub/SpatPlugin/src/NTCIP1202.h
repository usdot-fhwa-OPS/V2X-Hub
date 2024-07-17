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

#include <mutex>
#include <list>

#include <tmx/j2735_messages/SpatMessage.hpp>
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <ctime>
#include <ratio>
#include <PluginLog.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include "carma-clock/carma_clock.h"

using namespace std;

struct Ntcip1202Ext_PhaseTime
{
	uint8_t phaseNumber;
	uint16_t spatVehMinTimeToChange;
	uint16_t spatVehMaxTimeToChange;
	uint16_t spatPedMinTimeToChange;
	uint16_t spatPedMaxTimeToChange;
	uint16_t spatOvlpMinTimeToChange;
	uint16_t spatOvpMaxTimeToChange;
}__attribute__((packed));

struct Ntcip1202Ext
{
	int8_t header;
	int8_t numOfPhases;

	Ntcip1202Ext_PhaseTime phaseTimes[16];

	uint16_t phaseStatusGroupReds;
	uint16_t phaseStatusGroupYellows;
	uint16_t phaseStatusGroupGreens;
	uint16_t phaseStatusGroupDontWalks;
	uint16_t phaseStatusGroupPedClears;
	uint16_t phaseStatusGroupWalks;
	uint16_t overlapStatusGroupReds;
	uint16_t overlapStatusGroupYellows;
	uint16_t overlapStatusGroupGreens;
	uint16_t flashingOutputPhaseStatus;
	uint16_t flashingOutputOverlapStatus;
	uint8_t spatIntersectionStatus;
	uint8_t timebaseAscActionStatus;
	uint8_t spatDiscontinuoutChangeFlag;
	uint8_t spatMessageSeqCounter;
	uint8_t spatTimestamp_hr;
	uint8_t spatTimestamp_min;
	uint8_t spatTimestamp_sec;
	uint16_t spatTimestamp_msec;
	uint16_t spatPedestrianCall;
	uint16_t spatPedestrianDetect;
}__attribute__((packed));

struct SignalGroupMapping
{
	int SignalGroupId;
	int PhaseId;
	string Type;
};

class Ntcip1202
{
	public:
		void setSignalGroupMappingList(string json);

		void copyBytesIntoNtcip1202(char* buff, int numBytes);
		bool getPhaseRedStatus(int phaseNumber);
		bool getPhaseYellowStatus(int phaseNumber);
		bool getPhaseGreensStatus(int phaseNumber);

		bool getPhaseFlashingStatus(int phaseNumber);
		bool getOverlapFlashingStatus(int phaseNumber);

		bool getPhaseDontWalkStatus(int phaseNumber);
		bool getPhasePedClearsStatus(int phaseNumber);
		bool getPhaseWalkStatus(int phaseNumber);

		bool getOverlapRedStatus(int phaseNumber);
		bool getOverlapYellowStatus(int phaseNumber);
		bool getOverlapGreenStatus(int phaseNumber);

		bool getSpatPedestrianDetect(int phaseNumber);
		bool getSpatPedestrianCall(int phaseNumber);

		uint16_t getVehicleMinTime(int phaseNumber);
		uint16_t getVehicleMaxTime(int phaseNumber);

		uint16_t getPedMinTime(int phaseNumber);
		uint16_t getPedMaxTime(int phaseNumber);

		uint16_t getOverlapMinTime(int phaseNumber);
		uint16_t getOverlapMaxTime(int phaseNumber);

		long getAdjustedTime(unsigned int offset_tenthofSec, unsigned long msEpoch) const;

		bool isFlashingStatus();
		bool isPhaseFlashing();

		void ToJ2735SPAT(SPAT* spat, unsigned long msEpoch , const std::string &intersectionName, IntersectionID_t intersectionId);

		void printDebug();
	private:

		Ntcip1202Ext ntcip1202Data;
		std::map<uint8_t, int> _phaseToIndexMapping;


		list<SignalGroupMapping> signalGroupMappingList;

		int getVehicleSignalGroupForPhase(int phase);
		int getPedestrianSignalGroupForPhase(int phase);

		void populateVehicleSignalGroup(MovementState *movement, int phase, unsigned long msEpoch);
		void populatePedestrianSignalGroup(MovementState *movement, int phase, unsigned long msEpoch);
		void populateOverlapSignalGroup(MovementState *movement, int phase, unsigned long msEpoch);
};



