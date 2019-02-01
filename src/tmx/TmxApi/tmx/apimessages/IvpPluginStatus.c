/*
 * IvpPluginStatus.c
 *
 *  Created on: Jul 22, 2014
 *      Author: ivp
 */

#include "IvpPluginStatus.h"
#include <string.h>
#include <assert.h>


#define IVP_PLUGIN_STATUS_FIELD_KEY "key"
#define IVP_PLUGIN_STATUS_FIELD_VALUE "value"

inline int ivpPluginStatus_isStatusMsg(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return 0;
	return msg && msg->type && strcmp(msg->type, IVPMSG_TYPE_APIRESV_STATUS) == 0;
}

IvpMessage *ivpPluginStatus_createMsg(IvpPluginStatusCollection *collection)
{
	return ivpMsg_create(IVPMSG_TYPE_APIRESV_STATUS, NULL, IVP_ENCODING_JSON, IvpMsgFlags_None, collection);
}

IvpPluginStatusCollection *ivpPluginStatus_addStatusItem(IvpPluginStatusCollection *collection, const char *key, const char *value)
{
	if (collection != NULL)
	{
		assert(collection->type == cJSON_Array);
		if (collection->type != cJSON_Array)
			return collection;
	}
	else
		collection = cJSON_CreateArray();

	assert(collection != NULL);
	if (collection == NULL)
		return collection;

	if (value != NULL || key != NULL)
	{
		cJSON *item = cJSON_CreateObject();
		assert(item != NULL);
		if (item != NULL)
		{
			if (key != NULL) cJSON_AddStringToObject(item, IVP_PLUGIN_STATUS_FIELD_KEY, key);
			if (value != NULL) cJSON_AddStringToObject(item, IVP_PLUGIN_STATUS_FIELD_VALUE, value);

			cJSON_AddItemToArray(collection, item);
		}
	}

	return collection;
}

int ivpPluginStatus_getItemCount(IvpPluginStatusCollection *collection)
{
	if (collection != NULL)
	{
		assert(collection->type == cJSON_Array);
		if (collection->type != cJSON_Array)
			return 0;

		return cJSON_GetArraySize(collection);
	}
	return 0;
}

IvpPluginStatusItem *ivpPluginStatus_getItem(IvpPluginStatusCollection *collection, int index)
{
	assert(collection != NULL);
	assert(collection->type == cJSON_Array);
	assert(index < ivpPluginStatus_getItemCount(collection));
	if (collection == NULL || collection->type != cJSON_Array || index >= ivpPluginStatus_getItemCount(collection))
		return NULL;


	cJSON *item = cJSON_GetArrayItem(collection, index);
	assert(item != NULL);
	if (item == NULL)
		return NULL;

	IvpPluginStatusItem *results = calloc(1, sizeof(IvpPluginStatusItem));
	assert(results != NULL);
	if (results == NULL)
		return NULL;

	cJSON *keyField = cJSON_GetObjectItem(item, IVP_PLUGIN_STATUS_FIELD_KEY);
	cJSON *valueField = cJSON_GetObjectItem(item, IVP_PLUGIN_STATUS_FIELD_VALUE);

	//TODO: this can use cJSONxtra_tryGetStr()
	if (keyField != NULL && keyField->type == cJSON_String)
		results->key = strdup(keyField->valuestring);

	if (valueField != NULL && valueField->type == cJSON_String)
		results->value = strdup(valueField->valuestring);

	return results;
}

void ivpPluginStatus_destroyCollection(IvpPluginStatusCollection *collection)
{
	assert(collection != NULL);
	if (collection == NULL)
		return;

	cJSON_Delete(collection);
}

void ivpPluginStatus_destroyItem(IvpPluginStatusItem *item)
{
	assert(item != NULL);
	if (item == NULL)
		return;

	if (item->key != NULL) free(item->key);
	if (item->value != NULL) free(item->value);

	free(item);
}
