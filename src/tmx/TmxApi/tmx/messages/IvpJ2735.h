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
#define SAEJ2735_SPEC 2016
#endif

#include <DSRCmsgID.h>

#define IVPMSG_TYPE_J2735 "J2735"

#ifdef __cplusplus
extern "C"
{
#endif

// Include this for backwards compability
#if SAEJ2735_SPEC >= 2016
/* Dependencies */
typedef enum msgID {
	msgID_reserved	= 0,
	msgID_alaCarteMessage	= 1,
	msgID_basicSafetyMessageVerbose	= 3,
	msgID_intersectionCollisionAlert	= 6,
	msgID_uperFrame_D   = 17,
	msgID_mapData       = 18,
	msgID_signalPhaseAndTimingMessage   = 19,
	msgID_basicSafetyMessage    = 20,
	msgID_commonSafetyRequest   = 21,
	msgID_emergencyVehicleAlert = 22,
	msgID_intersectionCollision = 23,
	msgID_nmeaCorrections       = 24,
	msgID_probeDataManagement   = 25,
	msgID_probeVehicleData      = 26,
	msgID_roadSideAlert = 27,
	msgID_rtcmCorrections       = 28,
	msgID_signalRequestMessage  = 29,
	msgID_signalStatusMessage   = 30,
	msgID_travelerInformation   = 31,
	msgID_personalSafetyMessage = 32,
	msgID_roadSafetyMessage     = 33,
	msgID_roadWeatherMessage    = 34,
	msgID_probeDataConfigMessage        = 35,
	msgID_probeDataReportMessage        = 36,
	msgID_tollAdvertisementMessage      = 37,
	msgID_tollUsageMessage      = 38,
	msgID_tollUsageAckMessage   = 39,
	msgID_cooperativeControlMessage     = 40,
	msgID_sensorDataSharingMessage      = 41,
	msgID_maneuverSharingAndCoordinatingMessage = 42,
	msgID_roadGeometryAndAttributes     = 43,
	msgID_personalSafetyMessage2        = 44,
	msgID_trafficSignalPhaseAndTiming   = 45,
	msgID_signalControlAndPrioritizationRequest = 46,
	msgID_signalControlAndPrioritizationStatus  = 47,
	msgID_roadUserChargingConfigMessage = 48,
	msgID_roadUserChargingReportMessage = 49,
	msgID_trafficLightStatusMessage     = 50,
	msgID_testMessage00 = 240,
	msgID_testMessage01 = 241,
	msgID_testMessage02 = 242,
	msgID_testMessage03 = 243,
	msgID_testMessage04 = 244,
	msgID_testMessage05 = 245,
	msgID_testMessage06 = 246,
	msgID_testMessage07 = 247,
	msgID_testMessage08 = 248,
	msgID_testMessage09 = 249,
	msgID_testMessage10 = 250,
	msgID_testMessage11 = 251,
	msgID_testMessage12 = 252,
	msgID_testMessage13 = 253,
	msgID_testMessage14 = 254,
	msgID_testMessage15 = 255,
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
