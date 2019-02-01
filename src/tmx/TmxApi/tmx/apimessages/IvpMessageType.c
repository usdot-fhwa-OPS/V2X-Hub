/*
 * IvpMessageType.c
 *
 *  Created on: Aug 8, 2014
 *      Author: ivp
 */

#include <assert.h>
#include <string.h>
#include "IvpMessageType.h"

#define IVP_IVPMSGTYPE_FIELD_TYPE "type"
#define IVP_IVPMSGTYPE_FIELD_SUBTYPE "subtype"
#define IVP_IVPMSGTYPE_FIELD_DESCRIPTION "description"

IvpMessageTypeCollection *ivpMsgType_addEntryToCollection(IvpMessageTypeCollection *collection, const char *type, const char *subtype, const char *description)
{
	assert(type != NULL);
	assert(subtype != NULL);
	if (type == NULL || subtype == NULL)
		return collection;

	if (collection)
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

	cJSON *item = cJSON_CreateObject();
	assert(item != NULL);
	if (item != NULL)
	{
		cJSON_AddStringToObject(item, IVP_IVPMSGTYPE_FIELD_TYPE, type);
		cJSON_AddStringToObject(item, IVP_IVPMSGTYPE_FIELD_SUBTYPE, subtype);
		if (description != NULL) cJSON_AddStringToObject(item, IVP_IVPMSGTYPE_FIELD_DESCRIPTION, description);

		cJSON_AddItemToArray(collection, item);
	}

	return collection;
}

IvpError ivpMsgType_validateCollection(IvpMessageTypeCollection *collection)
{
	if (collection == NULL)
		return IVP_ERROR_INITIALIZER;

	if (collection->type != cJSON_Array)
		return ivpError_createError(IvpLogLevel_fatal, IvpError_msgTypeCollectionFormat, 0);

	int arraySize = cJSON_GetArraySize(collection);
	int i;
	for(i = 0; i < arraySize; i++)
	{
		cJSON *item = cJSON_GetArrayItem(collection, i);

		cJSON *typeField = cJSON_GetObjectItem(item, IVP_IVPMSGTYPE_FIELD_TYPE);
		if (typeField == NULL || typeField->type != cJSON_String)
			return ivpError_createError(IvpLogLevel_fatal, IvpError_msgTypeCollectionMissingType, 0);

		cJSON *subtypeField = cJSON_GetObjectItem(item, IVP_IVPMSGTYPE_FIELD_SUBTYPE);
		if (subtypeField == NULL || subtypeField->type != cJSON_String)
			return ivpError_createError(IvpLogLevel_fatal, IvpError_msgTypeCollectionMissingSubType, 0);

		cJSON *descriptionField = cJSON_GetObjectItem(item, IVP_IVPMSGTYPE_FIELD_DESCRIPTION);
		if (descriptionField != NULL && descriptionField->type != cJSON_String)
			return ivpError_createError(IvpLogLevel_fatal, IvpError_msgTypeCollectionFormat, 0);

		int j;
		for(j = 0; j < i; j++)
		{
			cJSON *checkedItem = cJSON_GetArrayItem(collection, j);
			if (strcmp(typeField->valuestring, cJSON_GetObjectItem(checkedItem, IVP_IVPMSGTYPE_FIELD_TYPE)->valuestring) == 0
					&& strcmp(subtypeField->valuestring, cJSON_GetObjectItem(checkedItem, IVP_IVPMSGTYPE_FIELD_SUBTYPE)->valuestring) == 0)
				return ivpError_createError(IvpLogLevel_fatal, IvpError_msgTypeCollectionDuplicate, 0);
		}
	}

	return IVP_ERROR_INITIALIZER;
}

int ivpMsgType_getEntryCount(IvpMessageTypeCollection *collection)
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

IvpMessageTypeEntry *ivpMsgType_getEntry(IvpMessageTypeCollection *collection, int index)
{
	assert(collection != NULL);
	assert(collection->type == cJSON_Array);
	assert(index < ivpMsgType_getEntryCount(collection));
	if (collection == NULL || collection->type != cJSON_Array || index >= ivpMsgType_getEntryCount(collection))
		return NULL;

	cJSON *item = cJSON_GetArrayItem(collection, index);
	assert(item != NULL);
	if (item == NULL)
		return NULL;

	IvpMessageTypeEntry *results = calloc(1, sizeof(IvpMessageTypeEntry));
	assert(results != NULL);
	if (results == NULL)
		return NULL;

	cJSON *typeField = cJSON_GetObjectItem(item, IVP_IVPMSGTYPE_FIELD_TYPE);
	cJSON *subtypeField = cJSON_GetObjectItem(item, IVP_IVPMSGTYPE_FIELD_SUBTYPE);
	cJSON *descriptionField = cJSON_GetObjectItem(item, IVP_IVPMSGTYPE_FIELD_DESCRIPTION);

	if (typeField != NULL && typeField->type == cJSON_String)
		results->type = strdup(typeField->valuestring);

	if (subtypeField != NULL && subtypeField->type == cJSON_String)
		results->subtype = strdup(subtypeField->valuestring);

	if (descriptionField != NULL && descriptionField->type == cJSON_String)
		results->description = strdup(descriptionField->valuestring);

	return results;
}

void ivpMsgType_destroyCollection(IvpMessageTypeCollection *collection)
{
	assert(collection != NULL);
	if (collection == NULL)
		return;

	cJSON_Delete(collection);
}

void ivpMsgType_destroyEntry(IvpMessageTypeEntry *entry)
{
	assert(entry != NULL);
	if (entry == NULL)
		return;

	if (entry->type != NULL) free(entry->type);
	if (entry->subtype != NULL) free(entry->subtype);
	if (entry->description != NULL) free(entry->description);

	free(entry);
}
