/*
 * IvpBattelleDsrc.h
 *
 *  Created on: Aug 7, 2014
 *      Author: ivp
 */

#ifndef IVPBATTELLEDSRC_H_
#define IVPBATTELLEDSRC_H_

#include "../tmx.h"
#include "../IvpMessage.h"
#include "../apimessages/IvpMessageType.h"
#include <stdio.h>

#define IVPMSG_TYPE_BATTELLEDSRC "Battelle-DSRC"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
	IvpBattelleDsrcMsgType_reserved = 0,
	IvpBattelleDsrcMsgType_GID,
	IvpBattelleDsrcMsgType_SPAT,
	IvpBattelleDsrcMsgType_GID_J2735_r41,
	IvpBattelleDsrcMsgType_SPAT_J2735_r41
} IvpBattelleDsrcMsgType;


typedef struct {
	uint8_t *msg;
	unsigned int msgLength;
	IvpBattelleDsrcMsgType msgType;
} IvpBattelleDsrcMsg;

int ivpBattelleDsrc_isBattelleDsrcMsg(IvpMessage *msg);

IvpMessage *ivpBattelleDsrc_createMsg(uint8_t *msg, unsigned int msgLength, IvpBattelleDsrcMsgType msgType, IvpMsgFlags flags);

IvpBattelleDsrcMsg *ivpBattelleDsrc_getBattelleDsrcMsg(IvpMessage *msg);

void ivpBattelleDsrc_destroyBattelleDsrcMsg(IvpBattelleDsrcMsg *msg);


#ifdef __cplusplus
}
#endif


#endif /* IVPBATTELLEDSRC_H_ */
