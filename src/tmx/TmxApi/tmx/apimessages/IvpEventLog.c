/*
 * IvpEventLog.c
 *
 *  Created on: Jul 31, 2014
 *      Author: ivp
 */

#include "IvpEventLog.h"
#include <string.h>
#include <assert.h>

#define IVP_EVENTLOG_FIELD_LEVEL "level"
#define IVP_EVENTLOG_FIELD_DESCRIPTION "description"


inline int ivpEventLog_isEventLogMsg(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return 0;
	return msg->type != NULL && strcmp(msg->type, IVPMSG_TYPE_APIRESV_EVENTLOG) == 0;
}

IvpMessage *ivpEventLog_createMsg(IvpLogLevel level, const char *description)
{
	assert(description != NULL);
	assert(description[0] != '\0');
	if (description == NULL || description[0] == '\0')
		return NULL;

	IvpMessage *results = NULL;

	cJSON *payload = cJSON_CreateObject();
	assert(payload != NULL);
	if (payload != NULL)
	{
		cJSON_AddNumberToObject(payload, IVP_EVENTLOG_FIELD_LEVEL, level);
		cJSON_AddStringToObject(payload, IVP_EVENTLOG_FIELD_DESCRIPTION, description);

		results = ivpMsg_create(IVPMSG_TYPE_APIRESV_EVENTLOG, NULL, IVP_ENCODING_JSON, IvpMsgFlags_None, payload);
		assert(results != NULL);

		cJSON_Delete(payload);
	}

	return results;
}

IvpEventLogEntry *ivpEventLog_getEventLogEntry(IvpMessage *msg)
{
	assert(msg != NULL);
	assert(ivpEventLog_isEventLogMsg(msg));
	assert(msg->payload != NULL);
	if (msg == NULL || !ivpEventLog_isEventLogMsg(msg) || msg->payload == NULL)
		return NULL;

	IvpEventLogEntry *results = calloc(1, sizeof(IvpEventLogEntry));
	assert(results != NULL);
	if (results != NULL)
	{
		int level;
		cJSONxtra_tryGetInt(msg->payload, IVP_EVENTLOG_FIELD_LEVEL, &level);
		results->level = level;
		cJSONxtra_tryGetStr(msg->payload, IVP_EVENTLOG_FIELD_DESCRIPTION, &results->description);
	}

	return results;
}

void ivpEventLog_destoryEventLogEntry(IvpEventLogEntry *entry)
{
	assert(entry != NULL);
	if(entry == NULL)
		return;

	if (entry->description != NULL) free(entry->description);
	free(entry);
}
