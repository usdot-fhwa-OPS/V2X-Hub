/*
 * IvpMessage.c
 *
 *  Created on: Jul 17, 2014
 *      Author: ivp
 */

#include "IvpMessage.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <pthread.h>

#define IVPMSG_FIELD_HEADER "header"
#define IVPMSG_FIELD_PAYLOAD "payload"
#define IVPMSG_HFIELD_TYPE "type"
#define IVPMSG_HFIELD_SUBTYPE "subtype"
#define IVPMSG_HFIELD_SOURCE "source"
#define IVPMSG_HFIELD_SOURCEID "sourceId"
#define IVPMSG_HFIELD_ENCODING "encoding"
#define IVPMSG_HFIELD_FLAGS "flags"
#define IVPMSG_HFIELD_DSRCMETADATA "dsrcMetadata"
#define IVPMSG_HFIELD_DSRCMETADATA_CHANNEL "channel"
#define IVPMSG_HFIELD_DSRCMETADATA_PSID "psid"
#define IVPMSG_HFIELD_TIMESTAMP "timestamp"

static pthread_mutex_t ivpMsg_numberOfMessages_mutex = PTHREAD_MUTEX_INITIALIZER;
static int ivpMsg_numberOfMessages = 0;

static void ivpMsg_refreshTimestamp(IvpMessage *msg);

IvpMessage *ivpMsg_create(const char *type, const char *subtype, const char *encoding, IvpMsgFlags flags, cJSON *payload)
{
	IvpMessage *newIvpMessage = calloc(1, sizeof(IvpMessage));
	assert(newIvpMessage != NULL);
	if (newIvpMessage == NULL)
		return NULL;

	if (type != NULL) newIvpMessage->type = strdup(type);
	if (subtype != NULL) newIvpMessage->subtype = strdup(subtype);
	newIvpMessage->source = NULL;
	if (encoding != NULL) newIvpMessage->encoding = strdup(encoding);
	if (payload != NULL) newIvpMessage->payload = cJSON_Duplicate(payload, 1);
	newIvpMessage->flags = flags;

	ivpMsg_refreshTimestamp(newIvpMessage);

	pthread_mutex_lock(&ivpMsg_numberOfMessages_mutex);
	ivpMsg_numberOfMessages++;
	pthread_mutex_unlock(&ivpMsg_numberOfMessages_mutex);
	return newIvpMessage;
}

IvpMessage *ivpMsg_addDsrcMetadata(IvpMessage *msg, int channel, int psid)
{
	assert(msg != NULL);
	if (msg == NULL)
		return msg;

	if (msg->dsrcMetadata == NULL)
		msg->dsrcMetadata = (IvpDsrcMetadata *)malloc(sizeof(IvpDsrcMetadata));

	assert(msg->dsrcMetadata != NULL);

	msg->dsrcMetadata->channel = channel;
	msg->dsrcMetadata->psid = psid;

	//msg->flags |= IvpMsgFlags_RouteDSRC;

	return msg;
}

IvpMessage *ivpMsg_parse(char *jsonmsg)
{
	assert(jsonmsg != NULL);
	if (jsonmsg == NULL)
		return NULL;

	IvpMessage *results = NULL;

	cJSON *root = cJSON_Parse(jsonmsg);
	if (root != NULL)
	{
		cJSON *header = cJSON_GetObjectItem(root, IVPMSG_FIELD_HEADER);
		if (header != NULL)
		{
			results = calloc(1, sizeof(IvpMessage));
			if (results != NULL)
			{
				cJSONxtra_tryGetStr(header, IVPMSG_HFIELD_TYPE, &results->type);
				cJSONxtra_tryGetStr(header, IVPMSG_HFIELD_SUBTYPE, &results->subtype);
				cJSONxtra_tryGetStr(header, IVPMSG_HFIELD_SOURCE, &results->source);
				cJSONxtra_tryGetUnsignedInt(header, IVPMSG_HFIELD_SOURCEID, &results->sourceId);
				cJSONxtra_tryGetStr(header, IVPMSG_HFIELD_ENCODING, &results->encoding);
				cJSONxtra_tryGetInt64(header, IVPMSG_HFIELD_TIMESTAMP, &results->timestamp);
				cJSONxtra_tryGetUnsignedInt(header, IVPMSG_HFIELD_FLAGS, &results->flags);

				cJSON *dsrcMetadata = cJSON_GetObjectItem(header, IVPMSG_HFIELD_DSRCMETADATA);
				if (dsrcMetadata != NULL)
				{
					results->dsrcMetadata = (IvpDsrcMetadata *)malloc(sizeof(IvpDsrcMetadata));
					assert(results->dsrcMetadata != NULL);

					cJSONxtra_tryGetInt(dsrcMetadata, IVPMSG_HFIELD_DSRCMETADATA_CHANNEL, &results->dsrcMetadata->channel);
					cJSONxtra_tryGetInt(dsrcMetadata, IVPMSG_HFIELD_DSRCMETADATA_PSID, &results->dsrcMetadata->psid);
				}

				results->payload = cJSON_DetachItemFromObject(root, IVPMSG_FIELD_PAYLOAD);

				pthread_mutex_lock(&ivpMsg_numberOfMessages_mutex);
				ivpMsg_numberOfMessages++;
				pthread_mutex_unlock(&ivpMsg_numberOfMessages_mutex);
			}
		}
		cJSON_Delete(root);
	}

	return results;
}

IvpMessage *ivpMsg_copy(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return NULL;

	IvpMessage *copiedMsg = calloc(1, sizeof(IvpMessage));
	assert(copiedMsg != NULL);
	if (copiedMsg == NULL)
		return NULL;

	if (msg->type != NULL) copiedMsg->type = strdup(msg->type);
	if (msg->subtype != NULL) copiedMsg->subtype = strdup(msg->subtype);
	if (msg->source != NULL) copiedMsg->source = strdup(msg->source);
	copiedMsg->sourceId = msg->sourceId;
	if (msg->encoding != NULL) copiedMsg->encoding = strdup(msg->encoding);
	copiedMsg->timestamp = msg->timestamp;
	copiedMsg->flags = msg->flags;
	if (msg->dsrcMetadata != NULL)
	{
		copiedMsg->dsrcMetadata = (IvpDsrcMetadata *)malloc(sizeof(IvpDsrcMetadata));
		assert(copiedMsg->dsrcMetadata != NULL);
		memcpy(copiedMsg->dsrcMetadata, msg->dsrcMetadata, sizeof(IvpDsrcMetadata));
	}
	if (msg->payload != NULL) copiedMsg->payload = cJSON_Duplicate(msg->payload, 1);

	pthread_mutex_lock(&ivpMsg_numberOfMessages_mutex);
	ivpMsg_numberOfMessages++;
	pthread_mutex_unlock(&ivpMsg_numberOfMessages_mutex);
	return copiedMsg;
}

void ivpMsg_refreshTimestamp(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	double millisecondsSinceEpoch =
	    (double)(tv.tv_sec) * 1000 +
	    (double)(tv.tv_usec) / 1000;

	msg->timestamp = millisecondsSinceEpoch;
}

char *ivpMsg_createJsonString(IvpMessage *msg, IvpMsg_FormatOptions options)
{
	assert(msg != NULL);
	if (msg == NULL)
		return NULL;

	char *results;

	cJSON *root = cJSON_CreateObject();
	assert(root != NULL);
	if (root != NULL)
	{
		cJSON *header = cJSON_CreateObject();
		assert(header != NULL);
		if (header != NULL)
		{
			if (msg->type != NULL) cJSON_AddStringToObject(header, IVPMSG_HFIELD_TYPE, msg->type);
			if (msg->subtype != NULL) cJSON_AddStringToObject(header, IVPMSG_HFIELD_SUBTYPE, msg->subtype);
			if (msg->source != NULL) cJSON_AddStringToObject(header, IVPMSG_HFIELD_SOURCE, msg->source);
			if (msg->sourceId != 0) cJSON_AddNumberToObject(header, IVPMSG_HFIELD_SOURCEID, msg->sourceId);
			if (msg->encoding != NULL) cJSON_AddStringToObject(header, IVPMSG_HFIELD_ENCODING, msg->encoding);
			cJSON_AddNumberToObject(header, IVPMSG_HFIELD_TIMESTAMP, msg->timestamp);
			cJSON_AddNumberToObject(header, IVPMSG_HFIELD_FLAGS, msg->flags);
			if (msg->dsrcMetadata != NULL)
			{
				cJSON *dsrcMetadata = cJSON_CreateObject();
				assert(dsrcMetadata != NULL);
				cJSON_AddNumberToObject(dsrcMetadata, IVPMSG_HFIELD_DSRCMETADATA_CHANNEL, msg->dsrcMetadata->channel);
				cJSON_AddNumberToObject(dsrcMetadata, IVPMSG_HFIELD_DSRCMETADATA_PSID, msg->dsrcMetadata->psid);
				cJSON_AddItemToObject(header, IVPMSG_HFIELD_DSRCMETADATA, dsrcMetadata);
			}
			cJSON_AddItemToObject(root, IVPMSG_FIELD_HEADER, header);
			if (msg->payload != NULL) cJSON_AddItemToObject(root, IVPMSG_FIELD_PAYLOAD, cJSON_Duplicate(msg->payload, 1));

			if (options & IvpMsg_FormatOptions_formatted)
				results = cJSON_Print(root);
			else
				results = cJSON_PrintUnformatted(root);
		}

		cJSON_Delete(root);
	}

	return results;
}

void ivpMsg_destroy(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return;

	if (msg->type != NULL) free(msg->type);
	if (msg->subtype != NULL) free(msg->subtype);
	if (msg->source != NULL) free(msg->source);
	if (msg->encoding != NULL) free(msg->encoding);
	if (msg->dsrcMetadata != NULL) free(msg->dsrcMetadata);
	if (msg->payload != NULL) cJSON_Delete(msg->payload);

	pthread_mutex_lock(&ivpMsg_numberOfMessages_mutex);
	ivpMsg_numberOfMessages--;
	pthread_mutex_unlock(&ivpMsg_numberOfMessages_mutex);
	free(msg);
}
