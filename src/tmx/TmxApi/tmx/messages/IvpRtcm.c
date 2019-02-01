/*
 * IvpRtcm.c
 *
 *  Created on: Aug 23, 2014
 *      Author: ivp
 */

#include "IvpRtcm.h"
#include <assert.h>
#include <string.h>

int ivpRtcm_isRtcmMsg(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return 0;
	return msg->type != NULL && strcmp(msg->type, IVPMSG_TYPE_RTCM) == 0;
}

IvpMessage *ivpRtcm_createMsg(IvpRtcmVersion version, const uint8_t *data, unsigned int length)
{
	assert(data != NULL);
	if (data == NULL)
		return NULL;

	assert(version >= IvpRtcmVersion_23 && version <= IvpRtcmVersion_30);
	if(!(version >= IvpRtcmVersion_23 && version <= IvpRtcmVersion_30))
		return NULL;

	IvpMessage *results = NULL;

	char payloadString[length * 2 + 1];
	int i;
	for(i = 0; i < length; i++)
	{
		snprintf(&payloadString[i * 2], 3, "%02x", data[i]);
	}

	cJSON *payload = cJSON_CreateString(payloadString);
	assert(payload != NULL);
	if (payload != NULL)
	{
		switch (version)
		{
		case IvpRtcmVersion_23:
			results = ivpMsg_create(IVPMSG_TYPE_RTCM, "2.3", IVP_ENCODING_STRING, IvpMsgFlags_None, payload);
			break;
		case IvpRtcmVersion_30:
			results = ivpMsg_create(IVPMSG_TYPE_RTCM, "3.0", IVP_ENCODING_STRING, IvpMsgFlags_None, payload);
			break;
		default:
			break;
		}
		assert(results != NULL);
	}

	return results;
}

IvpRtcmData *ivpRtcm_getRtcmData(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return NULL;

	if (msg->payload == NULL || msg->payload->type != cJSON_String)
		return NULL;

	if (msg->subtype == NULL)
		return NULL;

	IvpRtcmData *results = calloc(1, sizeof(IvpRtcmData));
	if (results != NULL)
	{
		if (strcmp(msg->subtype, "2.3") == 0)
			results->version = IvpRtcmVersion_23;
		else if (strcmp(msg->subtype, "3.0") == 0)
			results->version = IvpRtcmVersion_30;
		else
			results->version = IvpRtcmVersion_Unknown;

		int payloadLength = strlen(msg->payload->valuestring)/2;

		uint8_t *data = calloc(1, payloadLength);
		if (data != NULL)
		{
			int i;
			for(i = 0; i < payloadLength; i++)
				sscanf(msg->payload->valuestring + (2 * i), "%2hhx", &data[i]);

			results->data = data;
			results->dataLength = payloadLength;
		}
	}

	return results;
}

void ivpRtcm_destroyRtcmData(IvpRtcmData *data)
{
	assert(data != NULL);
	if (data == NULL)
		return;

	if (data->data != NULL) free(data->data);
	free(data);
}
