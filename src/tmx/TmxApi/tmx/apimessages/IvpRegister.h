/*
 * IvpRegister.h
 *
 *  Created on: Jul 22, 2014
 *      Author: ivp
 */

#ifndef IVPREGISTER_H_
#define IVPREGISTER_H_

#include "../tmx.h"
#include "IvpConfig.h"
#include "IvpMessageType.h"
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define IVPREGISTER_MANIFEST_FILE_NAME "manifest.json"

typedef struct {
	char *name;
	char *description;
	char *version;

	char *coreIpAddr;
	int corePort;

	IvpMessageTypeCollection *messageTypes;
	IvpConfigCollection *configuration;
} IvpManifest;

IvpMessage *ivpRegister_createMsgFromJson(cJSON *manifest);

int ivpRegister_isRegistrationMsg(IvpMessage *msg);

IvpManifest *ivpRegister_getManifest(IvpMessage *msg);
IvpManifest *ivpRegister_getManifestFromJson(cJSON *manifest);

IvpError ivpRegister_validateManifest(IvpManifest *manifest);

void ivpRegister_destroyManifest(IvpManifest *manifest);


#ifdef __cplusplus
}
#endif

#endif /* IVPREGISTER_H_ */
