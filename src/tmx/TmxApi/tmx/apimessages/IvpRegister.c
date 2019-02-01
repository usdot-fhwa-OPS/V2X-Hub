/*
 * IvpManifest.c
 *
 *  Created on: Jul 22, 2014
 *      Author: ivp
 */

#include "IvpRegister.h"
#include <stdlib.h>


IvpMessage *ivpRegister_createMsgFromJson(cJSON *manifest)
{
	assert(manifest != NULL);
	if (manifest == NULL)
		return NULL;

	return ivpMsg_create(IVPMSG_TYPE_APIRESV_REGISTER, NULL, "JSON", IvpMsgFlags_None, manifest);
}

inline int ivpRegister_isRegistrationMsg(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return 0;
	return msg->type != NULL && strcmp(msg->type, IVPMSG_TYPE_APIRESV_REGISTER) == 0;
}

IvpManifest *ivpRegister_getManifest(IvpMessage *msg)
{
	assert(msg != NULL);
	assert(msg->type != NULL);
	assert(ivpRegister_isRegistrationMsg(msg));
	assert(msg->payload != NULL);
	if (msg == NULL || msg->type == NULL || !ivpRegister_isRegistrationMsg(msg) || msg->payload == NULL)
		return NULL;

	return ivpRegister_getManifestFromJson(msg->payload);
}

IvpManifest *ivpRegister_getManifestFromJson(cJSON *manifest)
{
	assert(manifest != NULL);
	if (manifest == NULL)
		return NULL;

	IvpManifest *results = calloc(1, sizeof(IvpManifest));
	assert(results != NULL);
	if (results == NULL)
		return NULL;


	cJSON *name = cJSON_GetObjectItem(manifest, "name");
	if (name != NULL && name->type == cJSON_String)
		results->name = strdup(name->valuestring);

	cJSON *description = cJSON_GetObjectItem(manifest, "description");
	if (description != NULL && description->type == cJSON_String)
		results->description = strdup(description->valuestring);

	cJSON *version = cJSON_GetObjectItem(manifest, "version");
	if (version != NULL && version->type == cJSON_String)
		results->version = strdup(version->valuestring);


	cJSON *coreIpAddr = cJSON_GetObjectItem(manifest, "coreIpAddr");
	if (coreIpAddr != NULL && coreIpAddr->type == cJSON_String)
		results->coreIpAddr = strdup(coreIpAddr->valuestring);

	cJSON *corePort = cJSON_GetObjectItem(manifest, "corePort");
	if (corePort != NULL && corePort->type == cJSON_Number)
		results->corePort = corePort->valueint;

	results->configuration = cJSON_GetObjectItem(manifest, "configuration");
	if (results->configuration != NULL) results->configuration = cJSON_Duplicate(results->configuration, 1);

	results->messageTypes = cJSON_GetObjectItem(manifest, "messageTypes");
	if (results->messageTypes != NULL) results->messageTypes = cJSON_Duplicate(results->messageTypes, 1);

	return results;
}


IvpError ivpRegister_validateManifest(IvpManifest *manifest)
{
	assert(manifest != NULL);
	if (manifest == NULL)
		return ivpError_createError(IvpLogLevel_fatal, IvpError_assert, 0);

	if (manifest->name == NULL || manifest->name[0] == '\0')
		return ivpError_createError(IvpLogLevel_fatal, IvpError_pluginNoName, 0);

	if (manifest->configuration != NULL)
	{
		IvpError configError = ivpConfig_validate(manifest->configuration);
		if (configError.error != IvpError_none)
			return configError;
	}

	if (manifest->messageTypes != NULL)
	{
		IvpError messageTypeErr = ivpMsgType_validateCollection(manifest->messageTypes);
		if (messageTypeErr.error != IvpError_none)
			return messageTypeErr;
	}

	return IVP_ERROR_INITIALIZER;
}

void ivpRegister_destroyManifest(IvpManifest *manifest)
{
	assert(manifest != NULL);
	if (manifest == NULL)
		return;

	if (manifest->name != NULL) free(manifest->name);
	if (manifest->description != NULL) free(manifest->description);
	if (manifest->version != NULL) free(manifest->version);
	if (manifest->coreIpAddr != NULL) free(manifest->coreIpAddr);

	if (manifest->configuration != NULL) ivpConfig_destroyCollection(manifest->configuration);
	if (manifest->messageTypes != NULL) ivpMsgType_destroyCollection(manifest->messageTypes);

	free(manifest);
}
