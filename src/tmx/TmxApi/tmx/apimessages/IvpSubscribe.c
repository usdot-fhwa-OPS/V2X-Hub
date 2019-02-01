/*
 * IvpSubscribe.c
 *
 *  Created on: Jul 19, 2014
 *      Author: ivp
 */

#include "IvpSubscribe.h"


IvpMessage *ivpSubscribe_createMsg(IvpMsgFilter *filter)
{
	return ivpMsg_create(IVPMSG_TYPE_APIRESV_SUBSCRIBE, NULL, NULL, IvpMsgFlags_None, filter);
}

inline int ivpSubscribe_isSubscribeMsg(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return 0;
	return msg->type != NULL && strcmp(msg->type, IVPMSG_TYPE_APIRESV_SUBSCRIBE) == 0;
}

IvpMsgFilter *ivpSubscribe_addFilterEntry(IvpMsgFilter *filter, const char *type, const char *subtype)
{
	return ivpSubscribe_addFilterEntryWithFlagMask(filter, type, subtype, IvpMsgFlags_None);
}

IvpMsgFilter *ivpSubscribe_addFilterEntryWithFlagMask(IvpMsgFilter *filter, const char *type, const char *subtype, IvpMsgFlags flags)
{
	if (filter != NULL)
	{
		assert(filter->type == cJSON_Array);
		if (filter->type != cJSON_Array)
			return filter;
	}
	else
		filter = cJSON_CreateArray();

	assert(filter != NULL);
	if (filter == NULL)
		return filter;

	if (type != NULL)
	{
		cJSON *newFilter = cJSON_CreateObject();
		assert(newFilter != NULL);
		if (newFilter != NULL)
		{
			cJSON_AddStringToObject(newFilter, "type", type);
			if (subtype != NULL)
			{
				cJSON_AddStringToObject(newFilter, "subtype", subtype);
			}

			if (flags != IvpMsgFlags_None)
				cJSON_AddNumberToObject(newFilter, "flagmask", flags);
			cJSON_AddItemToArray(filter, newFilter);
		}
	}

	return filter;
}

void ivpSubscribe_destroyFilter(IvpMsgFilter *filter)
{
	//TODO: This collections should follow this model...
	if (filter)
		cJSON_Delete(filter);
}

int ivpSubscribe_getFilterEntryCount(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return 0;

	if (msg->payload == NULL || msg->payload->type != cJSON_Array)
		return 0;

	return cJSON_GetArraySize(msg->payload);
}
void ivpSubscribe_getFilterEntry(IvpMessage *msg, int index, char **typeout, char **subtypeout, IvpMsgFlags *flagsout)
{
	assert(msg != NULL);
	//TODO: missing many asserts...
	if (msg == NULL)
		return;

	*typeout = NULL;
	*subtypeout = NULL;
	*flagsout = IvpMsgFlags_None;


	cJSON *entry;
	if (index < ivpSubscribe_getFilterEntryCount(msg)
			&& (entry = cJSON_GetArrayItem(msg->payload, index)) != NULL)
	{
		cJSON *type = cJSON_GetObjectItem(entry, "type");
		cJSON *subtype = cJSON_GetObjectItem(entry, "subtype");
		cJSON *flagmask = cJSON_GetObjectItem(entry, "flagmask");

		if (type != NULL && type->type == cJSON_String)
			*typeout = type->valuestring;

		if (subtype != NULL && subtype->type == cJSON_String)
			*subtypeout = subtype->valuestring;

		if (flagmask != NULL && flagmask->type == cJSON_Number)
			*flagsout = flagmask->valueint;
	}
}
