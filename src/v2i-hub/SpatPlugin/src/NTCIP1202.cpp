/*
 * NTCIP1202.cpp
 *
 *  Created on: Apr 3, 2017
 *      Author: ivp
 */
#include <iostream>
#include <cstring>
#include "NTCIP1202.h"
#include <netinet/in.h>
#include <ctime>
#include <ratio>
#include <PluginLog.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost::property_tree;
using namespace tmx::utils;

#if SAEJ2735_SPEC < 63
#elif SAEJ2735_SPEC < 2020
typedef DSRC_MsgCount_t MsgCount_t;
typedef DSRC_TimeMark_t TimeMark_t;
#else
typedef Common_MsgCount_t MsgCount_t;
typedef SPAT_TimeMark_t TimeMark_t;
#endif
typedef DSecond_t DSecond2_t;

void Ntcip1202::setSignalGroupMappingList(string json)
{
	std::stringstream ss;
	ss<<json;

	ptree root;
	read_json(ss, root);

	for(auto & signalGroup : root.get_child("SignalGroups"))
	{
		int signalGroupId = signalGroup.second.get<int>("SignalGroupId");
		int phaseNumber = signalGroup.second.get<int>("Phase", 0);
		string typeName = signalGroup.second.get<string>("Type");

		PLOG(logDEBUG) <<"signalGroupId: "<<signalGroupId<<" phaseNumber: "<<phaseNumber<<" typeName: "<<typeName<<endl;

		SignalGroupMapping sgm;
		sgm.PhaseId = phaseNumber;
		sgm.SignalGroupId = signalGroupId;
		sgm.Type = typeName;

		signalGroupMappingList.push_back(sgm);
	}
}

void Ntcip1202::copyBytesIntoNtcip1202(char* buff, int numBytes)
{
	std::lock_guard<std::mutex> lock(_spat_lock);

	std::memcpy(&ntcip1202Data, buff, numBytes);

	unsigned char * vptr = (unsigned char *)&(ntcip1202Data);

	// map phase to array index for later use
	_phaseToIndexMapping.clear();
	for (int i = 0;i < 16;i++)
	{
		if (ntcip1202Data.phaseTimes[i].phaseNumber > 0)
		{
			_phaseToIndexMapping[ntcip1202Data.phaseTimes[i].phaseNumber] = i;
		}
	}

	//for (int i=0; i<numBytes; i++) { printf("%02x ", vptr[i]); }
	//printf("\n\n");

	printDebug();
}

uint16_t Ntcip1202::getVehicleMinTime(int phaseNumber)
{
	return ((ntohs(ntcip1202Data.phaseTimes[_phaseToIndexMapping[phaseNumber]].spatVehMinTimeToChange)));
}

uint16_t Ntcip1202::getVehicleMaxTime(int phaseNumber)
{
	return ((ntohs(ntcip1202Data.phaseTimes[_phaseToIndexMapping[phaseNumber]].spatVehMaxTimeToChange)));
}

uint16_t Ntcip1202::getPedMinTime(int phaseNumber)
{
	return ((ntohs(ntcip1202Data.phaseTimes[_phaseToIndexMapping[phaseNumber]].spatPedMinTimeToChange)));
}

uint16_t Ntcip1202::getPedMaxTime(int phaseNumber)
{
	return ((ntohs(ntcip1202Data.phaseTimes[_phaseToIndexMapping[phaseNumber]].spatPedMaxTimeToChange)));
}

uint16_t Ntcip1202::getOverlapMinTime(int phaseNumber)
{
	return ((ntohs(ntcip1202Data.phaseTimes[_phaseToIndexMapping[phaseNumber]].spatOvlpMinTimeToChange)));
}

uint16_t Ntcip1202::getOverlapMaxTime(int phaseNumber)
{
	return ((ntohs(ntcip1202Data.phaseTimes[_phaseToIndexMapping[phaseNumber]].spatOvpMaxTimeToChange)));
}

bool Ntcip1202::getPhaseFlashingStatus(int phaseNumber)
{
	return (bool)((ntohs(ntcip1202Data.flashingOutputPhaseStatus) >> (phaseNumber - 1))&0x01);
}

bool Ntcip1202::getOverlapFlashingStatus(int phaseNumber)
{
	return (bool)((ntohs(ntcip1202Data.flashingOutputOverlapStatus) >> (phaseNumber - 1))&0x01);
}

bool Ntcip1202::getPhaseRedStatus(int phaseNumber)
{
	return (bool)((ntohs(ntcip1202Data.phaseStatusGroupReds) >> (phaseNumber - 1))&0x01);
}

bool Ntcip1202::getPhaseYellowStatus(int phaseNumber)
{
	return (bool)((ntohs(ntcip1202Data.phaseStatusGroupYellows) >> (phaseNumber - 1))&0x01);
}

bool Ntcip1202::getPhaseGreensStatus(int phaseNumber)
{
	return (bool)((ntohs(ntcip1202Data.phaseStatusGroupGreens) >>(phaseNumber-1))&0x01);
}

bool Ntcip1202::getPhaseDontWalkStatus(int phaseNumber)
{
	return (bool)((ntohs(ntcip1202Data.phaseStatusGroupDontWalks) >> (phaseNumber - 1))&0x01);
}

bool Ntcip1202::getPhasePedClearsStatus(int phaseNumber)
{
	return (bool)((ntohs(ntcip1202Data.phaseStatusGroupPedClears) >> (phaseNumber - 1))&0x01);
}

bool Ntcip1202::getPhaseWalkStatus(int phaseNumber)
{
	return (bool)((ntohs(ntcip1202Data.phaseStatusGroupWalks) >> (phaseNumber - 1))&0x01);
}

bool Ntcip1202::getOverlapRedStatus(int phaseNumber)
{
	return (bool)((ntohs(ntcip1202Data.overlapStatusGroupReds) >> (phaseNumber - 1))&0x01);
}

bool Ntcip1202::getOverlapYellowStatus(int phaseNumber)
{
	return (bool)((ntohs(ntcip1202Data.overlapStatusGroupYellows) >> (phaseNumber - 1))&0x01);
}

bool Ntcip1202::getOverlapGreenStatus(int phaseNumber)
{
	return (bool)((ntohs(ntcip1202Data.overlapStatusGroupGreens) >> (phaseNumber - 1))&0x01);
}

bool Ntcip1202::getSpatPedestrianDetect(int phaseNumber)
{
	return (bool)((ntohs(ntcip1202Data.spatPedestrianDetect) >> (phaseNumber - 1))&0x01);
}

bool Ntcip1202::getSpatPedestrianCall(int phaseNumber)
{
	return (bool)((ntohs(ntcip1202Data.spatPedestrianCall) >> (phaseNumber - 1))&0x01);
}

bool Ntcip1202::isFlashingStatus()
{
	return (bool)(ntohs(ntcip1202Data.spatIntersectionStatus) >> 7);
}

bool Ntcip1202::isPhaseFlashing()
{
	return (bool)ntcip1202Data.flashingOutputPhaseStatus;
}

void Ntcip1202::printDebug()
{
	//printf("phase %d spatVehMinTimeToChange: %02x\r\n",1, ntcip1202Data.phaseTimes[1].spatVehMinTimeToChange);

	//printf("header: %02x\r\n", ntcip1202Data.header);
	//printf("phases: %02x\r\n", ntcip1202Data.numOfPhases);

	/*for(int i=0; i<16; i++)
	{
		printf("phase %d number: %02x\r\n",i, ntcip1202Data.phaseTimes[i].phaseNumber);

		printf("phase %d spatVehMinTimeToChange: %02x\r\n",i, ntcip1202Data.phaseTimes[i].spatVehMinTimeToChange);
		printf("phase %d spatVehMaxTimeToChange: %02x\r\n",i, ntcip1202Data.phaseTimes[i].spatVehMaxTimeToChange);
		printf("phase %d spatPedMinTimeToChange: %02x\r\n",i, ntcip1202Data.phaseTimes[i].spatPedMinTimeToChange);
		printf("phase %d spatPedMaxTimeToChange: %02x\r\n",i, ntcip1202Data.phaseTimes[i].spatPedMaxTimeToChange);
		printf("phase %d spatOvlpMinTimeToChange: %02x\r\n",i, ntcip1202Data.phaseTimes[i].spatOvlpMinTimeToChange);
		printf("phase %d spatOvpMaxTimeToChange: %02x\r\n",i, ntcip1202Data.phaseTimes[i].spatOvpMaxTimeToChange);
	}
*/
	for(int i=0; i<16; i++)
	{
		int phaseNum = i+1;
		PLOG(logDEBUG3) << "Phase " << phaseNum <<
				", Green " << getPhaseGreensStatus(phaseNum) <<
				", Yellow " << getPhaseYellowStatus(phaseNum) <<
				", Red " << getPhaseRedStatus(phaseNum) <<
				", Walk " << getPhaseWalkStatus(phaseNum) <<
				", Ped Clear " << getPhasePedClearsStatus(phaseNum) <<
				", Don't Walk " << getPhaseDontWalkStatus(phaseNum);
	}
}

bool Ntcip1202::ToJ2735r41SPAT(SPAT* spat, char* intersectionName, IntersectionID_t intersectionId)
{
	time_t epochSec = clock->nowInSeconds();
	struct tm utctime;
	gmtime_r( &epochSec, &utctime );

	// In SPAT, the time stamp is split into minute of the year and millisecond of the minute
	// Calculate the minute of the year
	long minOfYear = utctime.tm_min + (utctime.tm_hour * 60) + (utctime.tm_yday * 24 * 60);

	// Calculate the millisecond of the minute
	auto epochMs = clock->nowInMilliseconds();
	long msOfMin = 1000 * (epochSec % 60) + (epochMs % 1000);

	std::lock_guard<std::mutex> lock(_spat_lock);

	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SPAT, spat);

#if SAEJ2735_SPEC < 63
	spat->msgID = tmx::messages::SpatMessage::get_default_messageId();
#endif

	IntersectionState *intersection = (IntersectionState *)calloc(1, sizeof(IntersectionState));

	intersection->name = (DescriptiveName_t *) calloc(1, sizeof(DescriptiveName_t));

	intersection->name->size = strlen(intersectionName);
	intersection->name->buf = (uint8_t *) calloc(1, strlen(intersectionName));
	memcpy(intersection->name->buf, intersectionName, strlen(intersectionName));
	intersection->id.id = intersectionId;
	intersection->revision = (MsgCount_t) 1;

	intersection->moy = (MinuteOfTheYear_t *) calloc(1, sizeof(MinuteOfTheYear_t));
	*(intersection->moy) = minOfYear;

	intersection->timeStamp = (DSecond2_t *) calloc(1, sizeof(DSecond2_t));
	*(intersection->timeStamp) = msOfMin;

	uint16_t statusIntersection = ntcip1202Data.spatIntersectionStatus;

	intersection->status.buf = (uint8_t *)calloc(2, sizeof(uint8_t));
	intersection->status.size = 2 * sizeof(uint8_t);
	intersection->status.bits_unused = 0;
	intersection->status.buf[1] = statusIntersection;
	intersection->status.buf[0] = (statusIntersection >> 8);

	// Movement List
	for (int m = 0; m < 16; m++)
	{
		int phase = ntcip1202Data.phaseTimes[m].phaseNumber;

		for(std::list<SignalGroupMapping>::iterator it = signalGroupMappingList.begin(); it != signalGroupMappingList.end(); it++)
		{
			if(it->PhaseId == phase && boost::iequals(it->Type,"vehicle"))
			{
				if(it->SignalGroupId > 0)
				{
					MovementState *movement = (MovementState *) calloc(1, sizeof(MovementState));
					movement->signalGroup = it->SignalGroupId;

					populateVehicleSignalGroup(movement, phase);

					ASN_SEQUENCE_ADD(&intersection->states.list, movement);
				}

			}
		}

		for(std::list<SignalGroupMapping>::iterator it = signalGroupMappingList.begin(); it != signalGroupMappingList.end(); it++)
		{
			if(it->PhaseId == phase && boost::iequals(it->Type,"pedestrian"))
			{
				if(it->SignalGroupId > 0)
				{
					MovementState *movement = (MovementState *) calloc(1, sizeof(MovementState));
					movement->signalGroup = it->SignalGroupId;

					populatePedestrianSignalGroup(movement, phase);

					ASN_SEQUENCE_ADD(&intersection->states.list, movement);
				}
			}
		}

		for(std::list<SignalGroupMapping>::iterator it = signalGroupMappingList.begin(); it != signalGroupMappingList.end(); it++)
		{
			if(it->PhaseId == phase && boost::iequals(it->Type,"overlap"))
			{
				if(it->SignalGroupId > 0)
				{
					MovementState *movement = (MovementState *) calloc(1, sizeof(MovementState));
					movement->signalGroup = it->SignalGroupId;

					populateOverlapSignalGroup(movement, phase);

					ASN_SEQUENCE_ADD(&intersection->states.list, movement);
				}

			}
		}


	}
	ASN_SEQUENCE_ADD(&(spat->intersections.list), intersection);


	return true;
}

void Ntcip1202::populateVehicleSignalGroup(MovementState *movement, int phase)
{
	MovementEvent *stateTimeSpeed = (MovementEvent *) calloc(1, sizeof(MovementEvent));

	bool isFlashing = getPhaseFlashingStatus(phase);
	bool forceFlashing = isFlashingStatus();

	if(getPhaseRedStatus(phase))
	{
		PLOG(logDEBUG3) << "Phase " << phase <<
				" Red " << getPhaseRedStatus(phase) <<
				", isFlashing  " << isFlashing <<
				", forceFlashing " << forceFlashing ;
		if(isFlashing)
			stateTimeSpeed->eventState = MovementPhaseState_stop_Then_Proceed;
		else
			stateTimeSpeed->eventState = MovementPhaseState_stop_And_Remain;
	}
	else if(getPhaseYellowStatus(phase))
	{
		if(isFlashing)
			stateTimeSpeed->eventState = MovementPhaseState_caution_Conflicting_Traffic;
		else
		{
			//TODO Add protected and permissive
			stateTimeSpeed->eventState = MovementPhaseState_protected_clearance;
		}
	}
	else if(getPhaseGreensStatus(phase))
	{
		//TODO Add logic for permissive which I am not sure we can figure out based on what's in the Spat status being provide by the controller.
		stateTimeSpeed->eventState = MovementPhaseState_protected_Movement_Allowed;
	}
	else
	{
		stateTimeSpeed->eventState = MovementPhaseState_dark;
	}

	stateTimeSpeed->timing = (TimeChangeDetails * ) calloc(1, sizeof(TimeChangeDetails));
	stateTimeSpeed->timing->minEndTime =  getAdjustedTime(getVehicleMinTime(phase));

	if (getVehicleMaxTime(phase) > 0)
	{
		stateTimeSpeed->timing->maxEndTime = (TimeMark_t *) calloc(1, sizeof(TimeMark_t));
		*(stateTimeSpeed->timing->maxEndTime) = 36111; //getAdjustedTime(getVehicleMaxTime(phase));
	}

	//we only get a phase number 1-16 from ped detect, assume its a ped phase
//	if(getSpatPedestrianDetect(phase))
//	{
//		movement->maneuverAssistList = (ManeuverAssistList *) calloc(1, sizeof(ManeuverAssistList));
//		ConnectionManeuverAssist *pedDetect = (ConnectionManeuverAssist *) calloc(1, sizeof(ConnectionManeuverAssist));
//		pedDetect->connectionID = 0;
//		pedDetect->pedBicycleDetect = (PedestrianBicycleDetect_t *) calloc(1, sizeof(PedestrianBicycleDetect_t));
//
//		*(pedDetect->pedBicycleDetect) = 1;
//		ASN_SEQUENCE_ADD(&movement->maneuverAssistList->list, pedDetect);
//	}

	ASN_SEQUENCE_ADD(&movement->state_time_speed.list, stateTimeSpeed);
}

void Ntcip1202::populatePedestrianSignalGroup(MovementState *movement, int phase)
{
	MovementEvent *stateTimeSpeed = (MovementEvent *) calloc(1, sizeof(MovementEvent));

	if(getPhaseDontWalkStatus(phase))
	{
		stateTimeSpeed->eventState = MovementPhaseState_stop_And_Remain;
	}
	else if(getPhasePedClearsStatus(phase))
	{
		stateTimeSpeed->eventState = MovementPhaseState_protected_clearance;
	}
	else if(getPhaseWalkStatus(phase))
	{
		stateTimeSpeed->eventState = MovementPhaseState_permissive_Movement_Allowed;
	}
	else
	{
		stateTimeSpeed->eventState = MovementPhaseState_dark;
	}

	stateTimeSpeed->timing = (TimeChangeDetails * ) calloc(1, sizeof(TimeChangeDetails));
	stateTimeSpeed->timing->minEndTime =  getAdjustedTime(getPedMinTime(phase));

	if (ntcip1202Data.phaseTimes[phase].spatPedMaxTimeToChange > 0)
	{
		stateTimeSpeed->timing->maxEndTime = (TimeMark_t *) calloc(1, sizeof(TimeMark_t));
		*(stateTimeSpeed->timing->maxEndTime) = getAdjustedTime(getPedMaxTime(phase));
	}

	if(getSpatPedestrianDetect(phase))
	{
		movement->maneuverAssistList = (ManeuverAssistList *) calloc(1, sizeof(ManeuverAssistList));
		ConnectionManeuverAssist *pedDetect = (ConnectionManeuverAssist *) calloc(1, sizeof(ConnectionManeuverAssist));
		pedDetect->connectionID = 0;
		pedDetect->pedBicycleDetect = (PedestrianBicycleDetect_t *) calloc(1, sizeof(PedestrianBicycleDetect_t));

		*(pedDetect->pedBicycleDetect) = 1;
		ASN_SEQUENCE_ADD(&movement->maneuverAssistList->list, pedDetect);
	}
	ASN_SEQUENCE_ADD(&movement->state_time_speed.list, stateTimeSpeed);
}

void Ntcip1202::populateOverlapSignalGroup(MovementState *movement, int phase)
{
	MovementEvent *stateTimeSpeed = (MovementEvent *) calloc(1, sizeof(MovementEvent));

	bool isFlashing = getOverlapFlashingStatus(phase);


	if(getOverlapRedStatus(phase))
	{
		if(isFlashing)
			stateTimeSpeed->eventState = MovementPhaseState_stop_Then_Proceed;
		else
			stateTimeSpeed->eventState = MovementPhaseState_stop_And_Remain;
	}
	else if(getOverlapYellowStatus(phase))
	{
		if(isFlashing)
			stateTimeSpeed->eventState = MovementPhaseState_caution_Conflicting_Traffic;
		else
		{
			//TODO Add protected and permissive
			stateTimeSpeed->eventState = MovementPhaseState_protected_clearance;
		}
	}
	else if(getOverlapGreenStatus(phase))
	{
		//TODO Add protected and permissive
		stateTimeSpeed->eventState = MovementPhaseState_permissive_Movement_Allowed;
	}
	else
	{
		stateTimeSpeed->eventState = MovementPhaseState_dark;
	}

	stateTimeSpeed->timing = (TimeChangeDetails * ) calloc(1, sizeof(TimeChangeDetails));
	stateTimeSpeed->timing->minEndTime =  getAdjustedTime(getOverlapMinTime(phase));

	if (getOverlapMaxTime(phase) > 0)
	{
		stateTimeSpeed->timing->maxEndTime = (TimeMark_t *) calloc(1, sizeof(TimeMark_t));
		*(stateTimeSpeed->timing->maxEndTime) = getAdjustedTime(getOverlapMaxTime(phase));
	}

	//we only get a phase number 1-16 from ped detect, assume its a ped phase
//	if(getSpatPedestrianDetect(phase))
//	{
//		movement->maneuverAssistList = (ManeuverAssistList *) calloc(1, sizeof(ManeuverAssistList));
//		ConnectionManeuverAssist *pedDetect = (ConnectionManeuverAssist *) calloc(1, sizeof(ConnectionManeuverAssist));
//		pedDetect->connectionID = 0;
//		pedDetect->pedBicycleDetect = (PedestrianBicycleDetect_t *) calloc(1, sizeof(PedestrianBicycleDetect_t));
//
//		*(pedDetect->pedBicycleDetect) = 1;
//		ASN_SEQUENCE_ADD(&movement->maneuverAssistList->list, pedDetect);
//	}

	ASN_SEQUENCE_ADD(&movement->state_time_speed.list, stateTimeSpeed);
}

//this method assumes only one signal group per phase and will return last match, DONT USE
int Ntcip1202::getVehicleSignalGroupForPhase(int phase)
{
	int signalGroupId = -1;
	for(std::list<SignalGroupMapping>::iterator it = signalGroupMappingList.begin(); it != signalGroupMappingList.end(); it++)
	{
		if(it->PhaseId == phase && boost::iequals(it->Type,"vehicle"))
		{
			signalGroupId = it->SignalGroupId;
		}
	}

	return signalGroupId;
}

//this method assumes only one signal group per phase and will return last match, DONT USE
int Ntcip1202::getPedestrianSignalGroupForPhase(int phase)
{
	int signalGroupId = -1;
	for(std::list<SignalGroupMapping>::iterator it = signalGroupMappingList.begin(); it != signalGroupMappingList.end(); it++)
	{
		if(it->PhaseId == phase && boost::iequals(it->Type,"pedestrian"))
		{
			signalGroupId = it->SignalGroupId;
		}
	}

	return signalGroupId;
}

long Ntcip1202::getAdjustedTime(unsigned int offset_tenthofSec)
{
	// generate J2735 TimeMark which is:
	// Tenths of a second in the current or next hour
	// In units of 1/10th second from UTC time
	// first get new time is absolute milliseconds
	auto epochMs = clock->nowInMilliseconds() + (offset_tenthofSec * 100);
	// get minute and second of hour from UTC time
	time_t epochSec = epochMs / 1000;
	struct tm utctime;
	gmtime_r( &epochSec, &utctime );
	auto tenthsOfSecond = utctime.tm_min * 600;
	tenthsOfSecond += utctime.tm_sec * 10;
	tenthsOfSecond += (epochMs % 1000) / 100;
	return tenthsOfSecond;
}
