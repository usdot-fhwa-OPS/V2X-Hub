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
	DSRCmsgID_reserved	= 0,
	DSRCmsgID_alaCarteMessage	= 1,
	DSRCmsgID_basicSafetyMessage	= 2,
	DSRCmsgID_basicSafetyMessageVerbose	= 3,
	DSRCmsgID_commonSafetyRequest	= 4,
	DSRCmsgID_emergencyVehicleAlert	= 5,
	DSRCmsgID_intersectionCollisionAlert	= 6,
	DSRCmsgID_mapData	= 7,
	DSRCmsgID_nmeaCorrections	= 8,
	DSRCmsgID_probeDataManagement	= 9,
	DSRCmsgID_probeVehicleData	= 10,
	DSRCmsgID_roadSideAlert	= 11,
	DSRCmsgID_rtcmCorrections	= 12,
	DSRCmsgID_signalPhaseAndTimingMessage	= 13,
	DSRCmsgID_signalRequestMessage	= 14,
	DSRCmsgID_signalStatusMessage	= 15,
	DSRCmsgID_travelerInformation	= 16,
	DSRCmsgID_testmessage00	= 17,
	DSRCmsgID_testmessage01	= 18,
	DSRCmsgID_testmessage02	= 19,
	DSRCmsgID_testmessage03	= 20,
	DSRCmsgID_testmessage04	= 21,
	DSRCmsgID_testmessage05	= 22,
	DSRCmsgID_testMessage06	= 23,
	DSRCmsgID_testMessage07	= 24
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
