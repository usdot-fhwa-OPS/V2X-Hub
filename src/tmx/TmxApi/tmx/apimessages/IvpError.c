/*
 * IvpError.c
 *
 *  Created on: Jul 23, 2014
 *      Author: ivp
 */

#include "IvpError.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>


#define IVP_ERROR_FIELD_LEVEL "level"
#define IVP_ERROR_FIELD_ERROR "error"
#define IVP_ERROR_FIELD_SYSTEM_ERROR_NUMBER "error"

const IvpError IVP_ERROR_INITIALIZER = { .level = IvpLogLevel_debug, .error = IvpError_none, .sysErrNo = 0 };

IvpError ivpError_createError(IvpLogLevel level, IvpErrorNumber error, int sysErrNo)
{
	IvpError results = IVP_ERROR_INITIALIZER;
	results.level = level;
	results.error = error;
	results.sysErrNo = sysErrNo;

	return results;
}

inline int ivpError_isErrMsg(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return 0;
	return msg->type != NULL && strcmp(msg->type, IVPMSG_TYPE_APIRESV_ERROR) == 0;
}


IvpMessage *ivpError_createMsg(IvpError err)
{
	IvpMessage *results = NULL;
	cJSON *payload = cJSON_CreateObject();
	assert(payload != NULL);
	if (payload != NULL)
	{
		cJSON_AddNumberToObject(payload, IVP_ERROR_FIELD_LEVEL, err.level);
		cJSON_AddNumberToObject(payload, IVP_ERROR_FIELD_ERROR, err.error);
		cJSON_AddNumberToObject(payload, IVP_ERROR_FIELD_SYSTEM_ERROR_NUMBER, err.sysErrNo);

		results = ivpMsg_create(IVPMSG_TYPE_APIRESV_ERROR, NULL, IVP_ENCODING_JSON, IvpMsgFlags_None, payload);

		cJSON_Delete(payload);
	}

	return results;
}

IvpError ivpError_getError(IvpMessage *msg)
{
	assert(msg != NULL);
	assert(ivpError_isErrMsg(msg));
	if (msg == NULL || !ivpError_isErrMsg(msg))
		return IVP_ERROR_INITIALIZER;

	IvpError results = IVP_ERROR_INITIALIZER;

	if (msg->payload)
	{
		int level = 0;
		int error = 0;
		cJSONxtra_tryGetInt(msg->payload, IVP_ERROR_FIELD_LEVEL, &level);
		cJSONxtra_tryGetInt(msg->payload, IVP_ERROR_FIELD_ERROR, &error);
		cJSONxtra_tryGetInt(msg->payload, IVP_ERROR_FIELD_SYSTEM_ERROR_NUMBER, &results.sysErrNo);

		results.level = level;
		results.error = error;
	}

	return results;
}
