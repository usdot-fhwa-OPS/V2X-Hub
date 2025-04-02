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
	msgID_basicSafetyMessage	= 2,
	msgID_basicSafetyMessageVerbose	= 3,
	msgID_commonSafetyRequest	= 4,
	msgID_emergencyVehicleAlert	= 5,
	msgID_intersectionCollisionAlert	= 6,
	msgID_mapData	= 7,
	msgID_nmeaCorrections	= 8,
	msgID_probeDataManagement	= 9,
	msgID_probeVehicleData	= 10,
	msgID_roadSideAlert	= 11,
	msgID_rtcmCorrections	= 12,
	msgID_signalPhaseAndTimingMessage	= 13,
	msgID_signalRequestMessage	= 14,
	msgID_signalStatusMessage	= 15,
	msgID_travelerInformation	= 16,
	msgID_testmessage00	= 17,
	msgID_testmessage01	= 18,
	msgID_testmessage02	= 19,
	msgID_testmessage03	= 20,
	msgID_testmessage04	= 21,
	msgID_testmessage05	= 22,
	msgID_testMessage06	= 23,
	msgID_testMessage07	= 24
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
