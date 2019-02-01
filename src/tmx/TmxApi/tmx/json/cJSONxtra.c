/*
 * cJSONxtra.c
 *
 *  Created on: Jul 17, 2014
 *      Author: ivp
 */

#include "cJSONxtra.h"
#include <string.h>
#include <assert.h>

int cJSONxtra_tryGetStr(cJSON *root, const char *fieldName, char **out)
{
	assert(root != NULL);
	assert(fieldName != NULL);
	assert(out != NULL);
	assert(*out == NULL);

	if (root == NULL)
		return 0;

	cJSON *field = cJSON_GetObjectItem(root, fieldName);
	if (field == NULL || field->type != cJSON_String)
		return 0;

	*out = strdup(field->valuestring);
	return 1;
}

int cJSONxtra_tryGetUnsignedInt(cJSON *root, const char *fieldName, unsigned int *out)
{
	assert(root != NULL);
	assert(fieldName != NULL);
	assert(out != NULL);

	if (root == NULL)
		return 0;

	cJSON *field = cJSON_GetObjectItem(root, fieldName);
	if (field == NULL || field->type != cJSON_Number)
		return 0;

	*out = field->valueint;
	return 1;
}

int cJSONxtra_tryGetInt(cJSON *root, const char *fieldName, int *out)
{
	assert(root != NULL);
	assert(fieldName != NULL);
	assert(out != NULL);

	if (root == NULL)
		return 0;

	cJSON *field = cJSON_GetObjectItem(root, fieldName);
	if (field == NULL || field->type != cJSON_Number)
		return 0;

	*out = field->valueint;
	return 1;
}

int cJSONxtra_tryGetInt64(cJSON *root, const char *fieldName, uint64_t *out)
{
	assert(root != NULL);
	assert(fieldName != NULL);
	assert(out != NULL);

	if (root == NULL)
		return 0;

	cJSON *field = cJSON_GetObjectItem(root, fieldName);
	if (field == NULL || field->type != cJSON_Number)
		return 0;

	*out = field->valueint;
	return 1;
}

int cJSONxtra_tryGetDouble(cJSON *root, const char *fieldName, double *out)
{
	assert(root != NULL);
	assert(fieldName != NULL);
	assert(out != NULL);

	if (root == NULL)
		return 0;

	cJSON *field = cJSON_GetObjectItem(root, fieldName);
	if (field == NULL || field->type != cJSON_Number)
		return 0;

	*out = field->valuedouble;
	return 1;
}

