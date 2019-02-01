/*
 * IvpConfig.h
 *
 *  Created on: Jul 28, 2014
 *      Author: ivp
 */

#ifndef IVPCONFIG_H_
#define IVPCONFIG_H_

#include "../tmx.h"
#include "IvpError.h"

typedef cJSON IvpConfigCollection;

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
	char *key;
	char *value;
	char *defaultValue;
	char *description;
} IvpConfigItem;

IvpMessage *ivpConfig_createMsg(IvpConfigCollection *collection);

int ivpConfig_isConfigMsg(IvpMessage *msg);

IvpError ivpConfig_validate(IvpConfigCollection *collection);

int ivpConfig_getItemCount(IvpConfigCollection *collection);
IvpConfigItem *ivpConfig_getItem(IvpConfigCollection *collection, int index);

IvpConfigCollection *ivpConfig_addItemToCollection(IvpConfigCollection *collection, const char *key, const char *value, const char *defaultValue);
int ivpConfig_updateValueInCollection(IvpConfigCollection *collection, const char *key, const char *value);
char *ivpConfig_getCopyOfValueFromCollection(IvpConfigCollection *collection, const char *key);

void ivpConfig_destroyCollection(IvpConfigCollection *collection);
void ivpConfig_destroyConfigItem(IvpConfigItem *item);


#ifdef __cplusplus
}
#endif


#endif /* IVPCONFIG_H_ */
