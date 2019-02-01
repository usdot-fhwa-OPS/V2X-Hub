/*
 * IvpSignalControllerStatus.c
 *
 *  Created on: Sep 24, 2014
 *      Author: ivp
 */

#include "IvpSignalControllerStatus.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
int ivpSigCont_isSigContMsg(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return 0;
	return msg->type != NULL && strcmp(msg->type, IVPMSG_TYPE_SIGCONT) == 0;
}

IvpMessage *ivpSigCont_createMsg(int action)
{
		IvpMessage *results = NULL;

		char buffer[32];
		snprintf(buffer, 32, "%d", action);

		cJSON *payload = cJSON_CreateString(buffer);
		assert(payload != NULL);
		if (payload != NULL)
		{
			const char *subtype = "ACT";
			if (subtype != NULL)
			{
				results = ivpMsg_create(IVPMSG_TYPE_SIGCONT, subtype, IVP_ENCODING_STRING, IvpMsgFlags_None, payload);
				assert(results != NULL);
			}
		}
		cJSON_Delete(payload);
		return results;
}

int ivpSigCont_getIvpSignalControllerAction(IvpMessage *msg)
{
	assert(msg != NULL);
		if (msg == NULL)
			return -1;

		char *results = NULL;

		if (msg->payload != NULL && msg->payload->type == cJSON_String)
			results = strdup(msg->payload->valuestring);

		int action = atoi(results);


		return action;
}



