/*
 * IvpConfig.c
 *
 *  Created on: Jul 28, 2014
 *      Author: ivp
 */

#include "IvpConfig.h"
#include <assert.h>
#include <string.h>

#define IVP_CONFIG_FIELD_KEY "key"
#define IVP_CONFIG_FIELD_VALUE "value"
#define IVP_CONFIG_FIELD_DEFAULT "default"
#define IVP_CONFIG_FIELD_DESCRIPTION "description"


IvpMessage *ivpConfig_createMsg(IvpConfigCollection *collection)
{
	return ivpMsg_create(IVPMSG_TYPE_APIRESV_CONFIG, NULL, IVP_ENCODING_JSON, IvpMsgFlags_None, collection);
}

inline int ivpConfig_isConfigMsg(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return 0;
	return msg->type && strcmp(msg->type, IVPMSG_TYPE_APIRESV_CONFIG) == 0;
}

IvpError ivpConfig_validate(IvpConfigCollection *collection)
{
	if (collection == NULL)
		return IVP_ERROR_INITIALIZER;

	if (collection->type != cJSON_Array)
		return ivpError_createError(IvpLogLevel_fatal, IvpError_configFormat, 0);

	int arraySize = cJSON_GetArraySize(collection);
	int i;
	for(i = 0; i < arraySize; i++)
	{
		cJSON *item = cJSON_GetArrayItem(collection, i);

		cJSON *key = cJSON_GetObjectItem(item, IVP_CONFIG_FIELD_KEY);
		if (key == NULL || key->type != cJSON_String)
			return ivpError_createError(IvpLogLevel_fatal, IvpError_configMissingKey, 0);

		cJSON *defaultValue = cJSON_GetObjectItem(item, IVP_CONFIG_FIELD_DEFAULT);
		if (defaultValue == NULL || defaultValue->type != cJSON_String)
			return ivpError_createError(IvpLogLevel_fatal, IvpError_configMissingDefault, 0);

		cJSON *description = cJSON_GetObjectItem(item, IVP_CONFIG_FIELD_DESCRIPTION);
		if (description != NULL && description->type != cJSON_String)
			return ivpError_createError(IvpLogLevel_fatal, IvpError_configFormat, 0);

		int j;
		for(j = 0; j < i; j++)
		{
			cJSON *checkedItem = cJSON_GetArrayItem(collection, j);
			if (strcmp(cJSON_GetObjectItem(item, IVP_CONFIG_FIELD_KEY)->valuestring, cJSON_GetObjectItem(checkedItem, IVP_CONFIG_FIELD_KEY)->valuestring) == 0)
				return ivpError_createError(IvpLogLevel_fatal, IvpError_configDuplicateKey, 0);
		}
	}

	return IVP_ERROR_INITIALIZER;
}

int ivpConfig_getItemCount(IvpConfigCollection *collection)
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

IvpConfigItem *ivpConfig_getItem(IvpConfigCollection *collection, int index)
{
	assert(collection != NULL);
	assert(collection->type == cJSON_Array);
	assert(index < ivpConfig_getItemCount(collection));
	if (collection == NULL || collection->type != cJSON_Array || index >= ivpConfig_getItemCount(collection))
		return NULL;

	cJSON *item = cJSON_GetArrayItem(collection, index);
	assert(item != NULL);
	if (item == NULL)
		return NULL;

	IvpConfigItem *results = calloc(1, sizeof(IvpConfigItem));
	assert(results != NULL);
	if (results == NULL)
		return NULL;

	cJSON *keyField = cJSON_GetObjectItem(item, IVP_CONFIG_FIELD_KEY);
	cJSON *valueField = cJSON_GetObjectItem(item, IVP_CONFIG_FIELD_VALUE);
	cJSON *defaultValueField = cJSON_GetObjectItem(item, IVP_CONFIG_FIELD_DEFAULT);
	cJSON *descriptionField = cJSON_GetObjectItem(item, IVP_CONFIG_FIELD_DESCRIPTION);

	if (keyField != NULL && keyField->type == cJSON_String)
		results->key = strdup(keyField->valuestring);

	if (valueField != NULL && valueField->type == cJSON_String)
		results->value = strdup(valueField->valuestring);

	if (defaultValueField != NULL && defaultValueField->type == cJSON_String)
		results->defaultValue = strdup(defaultValueField->valuestring);

	if (descriptionField != NULL && descriptionField->type == cJSON_String)
		results->description = strdup(descriptionField->valuestring);

	return results;
}

IvpConfigCollection *ivpConfig_addItemToCollection(IvpConfigCollection *collection, const char *key, const char *value, const char *defaultValue)
{
	assert(key != NULL);
	if (key == NULL)
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

	if (key != NULL)
	{
		if (value != NULL || defaultValue != NULL)
		{
			cJSON *item = cJSON_CreateObject();
			assert(item != NULL);
			if (item != NULL)
			{
				cJSON_AddStringToObject(item, IVP_CONFIG_FIELD_KEY, key);
				if (value != NULL) cJSON_AddStringToObject(item, IVP_CONFIG_FIELD_VALUE, value);
				if (defaultValue != NULL) cJSON_AddStringToObject(item, IVP_CONFIG_FIELD_DEFAULT, defaultValue);

				cJSON_AddItemToArray(collection, item);
			}
		}
	}

	return collection;
}

int ivpConfig_updateValueInCollection(IvpConfigCollection *collection, const char *key, const char *value)
{
	//TODO HACK this needs cleaned up.
	assert(key != NULL);
	assert(value != NULL);
	if (key == NULL || value == NULL)
		return 0;

	int arraySize = ivpConfig_getItemCount(collection);
	int i;
	for(i = 0; i < arraySize; i++)
	{
		cJSON *item = cJSON_GetArrayItem(collection, i);
		assert(item != NULL);

		cJSON *keyField = cJSON_GetObjectItem(item, IVP_CONFIG_FIELD_KEY);
		assert(keyField != NULL);
		assert(keyField->type == cJSON_String);
		if (keyField == NULL || keyField->type != cJSON_String)
			continue;

		if (strcmp(key, keyField->valuestring) == 0)
		{
			cJSON *newValue = cJSON_CreateString(value);
			assert(newValue != NULL);
			if (newValue != NULL)
			{
				cJSON *existingItem = cJSON_GetObjectItem(item, IVP_CONFIG_FIELD_VALUE);
				if (existingItem == NULL)
				{
					cJSON_AddItemToObject(item, IVP_CONFIG_FIELD_VALUE, newValue);
					//TODO: this doesn't compare the new value vs the default value.  would still return 1 even though the value hasn't technically changed.
					return 1;
				}
				else
				{
					if (existingItem->type == cJSON_String && strcmp(existingItem->valuestring, newValue->valuestring) == 0)
					{
						cJSON_Delete(newValue);
					}
					else
					{
						cJSON_ReplaceItemInObject(item, IVP_CONFIG_FIELD_VALUE, newValue);
						return 1;
					}
				}
			}
			break;
		}
	}

	return 1;
}

char *ivpConfig_getCopyOfValueFromCollection(IvpConfigCollection *collection, const char *key)
{
	assert(collection != NULL);
	assert(collection->type == cJSON_Array);
	if (collection == NULL || collection->type != cJSON_Array)
		return NULL;

	int arraySize = cJSON_GetArraySize(collection);
	int i;
	for(i = 0; i < arraySize; i++)
	{
		cJSON *item = cJSON_GetArrayItem(collection, i);

		cJSON *keyField = cJSON_GetObjectItem(item, IVP_CONFIG_FIELD_KEY);
		assert(keyField != NULL);
		assert(keyField->type == cJSON_String);
		if (keyField == NULL || keyField->type != cJSON_String)
			continue;

		if (strcmp(keyField->valuestring, key) != 0)
			continue;

		cJSON *valueField = cJSON_GetObjectItem(item, IVP_CONFIG_FIELD_VALUE);
		if (valueField != NULL)
		{
			assert(valueField->type == cJSON_String);
			if (valueField->type != cJSON_String)
				return NULL;
			return strdup(valueField->valuestring);
		}
		else
		{
			cJSON *defaultValueField = cJSON_GetObjectItem(item, IVP_CONFIG_FIELD_DEFAULT);
			assert(defaultValueField != NULL);
			assert(defaultValueField->type == cJSON_String);
			if (defaultValueField == NULL || defaultValueField->type != cJSON_String)
				return NULL;
			return strdup(defaultValueField->valuestring);
		}
	}

	return NULL;
}

void ivpConfig_destroyCollection(IvpConfigCollection *collection)
{
	assert(collection != NULL);
	if (collection == NULL)
		return;

	cJSON_Delete(collection);
}

void ivpConfig_destroyConfigItem(IvpConfigItem *item)
{
	assert(item != NULL);
	if (item == NULL)
		return;

	if (item->key != NULL) free(item->key);
	if (item->value != NULL) free(item->value);
	if (item->defaultValue != NULL) free(item->defaultValue);
	if (item->description != NULL) free(item->description);

	free(item);
}
