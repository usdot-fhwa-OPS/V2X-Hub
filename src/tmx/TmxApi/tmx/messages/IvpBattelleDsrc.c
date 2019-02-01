/*
 * IvpBattelleDsrc.c
 *
 *  Created on: Aug 7, 2014
 *      Author: ivp
 */

#include "IvpBattelleDsrc.h"

#include <assert.h>
#include <string.h>

int ivpBattelleDsrc_isBattelleDsrcMsg(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return 0;
	return msg->type != NULL && strcmp(msg->type, IVPMSG_TYPE_BATTELLEDSRC) == 0;
}

IvpMessage *ivpBattelleDsrc_createMsg(uint8_t *msg, unsigned int msgLength, IvpBattelleDsrcMsgType msgType, IvpMsgFlags flags)
{
	assert(msg != NULL);
	if (msg == NULL)
		return NULL;

	char buf[msgLength * 2 + 1];

	int i;
	for(i = 0; i < msgLength; i++)
	{
		snprintf(&buf[i*2], 3, "%02x", (uint8_t)msg[i]);
	}

	IvpMessage *results = NULL;

	cJSON *payload = cJSON_CreateString(buf);
	assert(payload != NULL);
	if (payload != NULL)
	{
		char *subtype;
		switch(msgType)
		{
			case IvpBattelleDsrcMsgType_GID:
				subtype = "GID";
				break;
			case IvpBattelleDsrcMsgType_GID_J2735_r41:
				subtype = "GID_J2735_r41";
				break;
			case IvpBattelleDsrcMsgType_SPAT:
				subtype = "SPAT";
				break;
			case IvpBattelleDsrcMsgType_SPAT_J2735_r41:
				subtype = "SPAT_J2735_r41";
				break;
			default:
				subtype = NULL;
				break;
		}

		assert(subtype != NULL);
		if (subtype != NULL)
		{
			results = ivpMsg_create(IVPMSG_TYPE_BATTELLEDSRC, subtype, IVP_ENCODING_BYTEARRAY, flags, payload);
			assert(results != NULL);
		}
	}

	return results;
}

IvpBattelleDsrcMsg *ivpBattelleDsrc_getBattelleDsrcMsg(IvpMessage *msg)
{
	assert(msg != NULL);
	assert(ivpBattelleDsrc_isBattelleDsrcMsg(msg));
	assert(msg->subtype != NULL);
	if (msg == NULL || !ivpBattelleDsrc_isBattelleDsrcMsg(msg) || msg->subtype == NULL)
		return NULL;

	//TODO some of these checks (including subtype from above) can be moved into isJ2735Msg()
	if (msg->payload == NULL)
		return NULL;

	assert(msg->payload->type == cJSON_String);
	if (msg->payload->type != cJSON_String)
		return NULL;

	IvpBattelleDsrcMsg *results = (IvpBattelleDsrcMsg *)calloc(1, sizeof(IvpBattelleDsrcMsg));
	assert(results != NULL);
	if (results != NULL)
	{
	
		results->msgLength = strlen(msg->payload->valuestring)/2;
		results->msg = (uint8_t *)malloc(results->msgLength);
		assert(results->msg != NULL);
		if (results->msg != NULL)
		{

			if (strcmp(msg->subtype, "GID") == 0) results->msgType = IvpBattelleDsrcMsgType_GID;
			else if (strcmp(msg->subtype, "SPAT") == 0) results->msgType = IvpBattelleDsrcMsgType_SPAT;
			else results->msgType = IvpBattelleDsrcMsgType_reserved;

			int i;
			for(i = 0; i < results->msgLength; i++)
				sscanf(&msg->payload->valuestring[2 * i], "%2hhx", &results->msg[i]);

			return results;
		}

		free(results);
	}

	return NULL;
}

void ivpBattelleDsrc_destroyBattelleDsrcMsg(IvpBattelleDsrcMsg *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return;

	if (msg->msg != NULL) free(msg->msg);
	free(msg);
}
