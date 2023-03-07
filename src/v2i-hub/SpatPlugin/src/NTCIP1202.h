/*
 * NTCIP1202.h
 *
 *  Created on: Apr 3, 2017
 *      Author: ivp
 */

#ifndef SRC_NTCIP1202_H_
#define SRC_NTCIP1202_H_

#include <mutex>
#include <list>

#include <tmx/j2735_messages/SpatMessage.hpp>

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
		inline explicit Ntcip1202(std::shared_ptr<fwha_stol::lib::time::CarmaClock> clock) :
			clock(clock) {};
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

		long getAdjustedTime(unsigned int offset);

		bool isFlashingStatus();
		bool isPhaseFlashing();

		bool ToJ2735r41SPAT(SPAT* spat, char* intersectionName, IntersectionID_t intersectionId);

		void printDebug();
	private:
		std::shared_ptr<fwha_stol::lib::time::CarmaClock> clock;

		Ntcip1202Ext ntcip1202Data;
		std::map<uint8_t, int> _phaseToIndexMapping;

		std::mutex _spat_lock;

		list<SignalGroupMapping> signalGroupMappingList;

		int getVehicleSignalGroupForPhase(int phase);
		int getPedestrianSignalGroupForPhase(int phase);

		void populateVehicleSignalGroup(MovementState *movement, int phase);
		void populatePedestrianSignalGroup(MovementState *movement, int phase);
		void populateOverlapSignalGroup(MovementState *movement, int phase);
};



#endif /* SRC_NTCIP1202_H_ */
