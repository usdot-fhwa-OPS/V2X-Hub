/*
 * IvpJ2735.h
 *
 *  Created on: Aug 7, 2014
 *      Author: ivp
 */

#ifndef IVPJ2735_H_
#define IVPJ2735_H_

#include "../tmx.h"
#include "../IvpMessage.h"
#include "../apimessages/IvpMessageType.h"
#include <stdio.h>

#ifndef SAEJ2735_SPEC
#define SAEJ2735_SPEC 2024
#endif

#include <DSRCmsgID.h>

#define IVPMSG_TYPE_J2735 "J2735"

#ifdef __cplusplus
extern "C"
{
#endif

// Include this for backwards compability
#if SAEJ2735_SPEC >= 63
/* Dependencies */
typedef enum DSRCmsgID {
	DSRCmsgID_reservedMessageId_D	= 0,
	DSRCmsgID_alaCarteMessage_D	= 1,
	DSRCmsgID_basicSafetyMessage_D	= 2,
	DSRCmsgID_basicSafetyMessageVerbose_D	= 3,
	DSRCmsgID_commonSafetyRequest_D	= 4,
	DSRCmsgID_emergencyVehicleAlert_D	= 5,
	DSRCmsgID_intersectionCollision_D	= 6,
	DSRCmsgID_mapData_D	= 7,
	DSRCmsgID_nmeaCorrections_D	= 8,
	DSRCmsgID_probeDataManagement_D	= 9,
	DSRCmsgID_probeVehicleData_D	= 10,
	DSRCmsgID_roadSideAlert_D	= 11,
	DSRCmsgID_rtcmCorrections_D	= 12,
	DSRCmsgID_signalPhaseAndTimingMessage_D	= 13,
	DSRCmsgID_signalRequestMessage_D	= 14,
	DSRCmsgID_signalStatusMessage_D	= 15,
	DSRCmsgID_travelerInformation_D	= 16,
	DSRCmsgID_uperFrame_D	= 17,
	DSRCmsgID_mapData	= 18,
	DSRCmsgID_signalPhaseAndTimingMessage	= 19,
	DSRCmsgID_basicSafetyMessage	= 20,
	DSRCmsgID_commonSafetyRequest	= 21,
	DSRCmsgID_emergencyVehicleAlert	= 22,
	DSRCmsgID_intersectionCollision	= 23,
	DSRCmsgID_nmeaCorrections	= 24,
	DSRCmsgID_probeDataManagement	= 25,
	DSRCmsgID_probeVehicleData	= 26,
	DSRCmsgID_roadSideAlert	= 27,
	DSRCmsgID_rtcmCorrections	= 28,
	DSRCmsgID_signalRequestMessage	= 29,
	DSRCmsgID_signalStatusMessage	= 30,
	DSRCmsgID_travelerInformation	= 31,
	DSRCmsgID_personalSafetyMessage	= 32,
	DSRCmsgID_roadSafetyMessage	= 33,
	DSRCmsgID_roadWeatherMessage	= 34,
	DSRCmsgID_probeDataConfigMessage	= 35,
	DSRCmsgID_probeDataReportMessage	= 36,
	DSRCmsgID_tollAdvertisementMessage	= 37,
	DSRCmsgID_tollUsageMessage	= 38,
	DSRCmsgID_tollUsageAckMessage	= 39,
	DSRCmsgID_cooperativeControlMessage	= 40,
	DSRCmsgID_sensorDataSharingMessage	= 41,
	DSRCmsgID_maneuverSharingAndCoordinatingMessage	= 42,
	DSRCmsgID_roadGeometryAndAttributes	= 43,
	DSRCmsgID_personalSafetyMessage2	= 44,
	DSRCmsgID_trafficSignalPhaseAndTiming	= 45,
	DSRCmsgID_signalControlAndPrioritizationRequest	= 46,
	DSRCmsgID_signalControlAndPrioritizationStatus	= 47,
	DSRCmsgID_roadUserChargingConfigMessage	= 48,
	DSRCmsgID_roadUserChargingReportMessage	= 49,
	DSRCmsgID_trafficLightStatusMessage	= 50,
	DSRCmsgID_testMessage00	= 240,
	DSRCmsgID_testMessage01	= 241,
	DSRCmsgID_testMessage02	= 242,
	DSRCmsgID_testMessage03	= 243,
	DSRCmsgID_testMessage04	= 244,
	DSRCmsgID_testMessage05	= 245,
	DSRCmsgID_testMessage06	= 246,
	DSRCmsgID_testMessage07	= 247,
	DSRCmsgID_testMessage08	= 248,
	DSRCmsgID_testMessage09	= 249,
	DSRCmsgID_testMessage10	= 250,
	DSRCmsgID_testMessage11	= 251,
	DSRCmsgID_testMessage12	= 252,
	DSRCmsgID_testMessage13	= 253,
	DSRCmsgID_testMessage14	= 254,
	DSRCmsgID_testMessage15	= 255

	/*
	 * Enumeration is extensible
	 */
} e_DSRCmsgID;

#endif

typedef struct {
	void *msgStructure;
	e_DSRCmsgID msgId;
} IvpJ2735Msg;


typedef struct {
	uint8_t *msg;
	unsigned int msgLength;
	e_DSRCmsgID msgId;
} IvpJ2735EncodedMsg;

int ivpJ2735_isJ2735Msg(IvpMessage *msg);

IvpMessageTypeCollection *ivpJ2735_addMsgTypeToCollection(IvpMessageTypeCollection *collection, e_DSRCmsgID msgId);

IvpMessage *ivpJ2735_createMsg(void *msgStructure, e_DSRCmsgID msgId, IvpMsgFlags flags);
IvpMessage *ivpJ2735_createMsgFromEncoded(uint8_t *msg, unsigned int msgLength, IvpMsgFlags flags);
IvpMessage *ivpJ2735_createMsgFromEncodedwType(uint8_t *msg, unsigned int msgLength, IvpMsgFlags flags, const char * msgType);

IvpJ2735Msg *ivpJ2735_getJ2735Msg(IvpMessage *msg);
IvpJ2735EncodedMsg *ivpJ2735_getJ2735EncodedMsg(IvpMessage *msg);


void ivpJ2735_destroyJ2735Msg(IvpJ2735Msg *msg);
void ivpJ2735_destroyJ2735EncodedMsg(IvpJ2735EncodedMsg *msg);

IvpJ2735EncodedMsg *ivpJ2735_getJ2735EncodedMsg(IvpMessage *msg);

IvpJ2735Msg *ivpJ2735_decode(uint8_t *msg, unsigned int msgLength);

void ivpJ2735_fprint(FILE *stream, IvpJ2735Msg *msg);

#ifdef __cplusplus
}
#endif


#endif /* IVPJ2735_H_ */
