/*
 * IvpPluginStatus.h
 *
 *  Created on: Jul 20, 2014
 *      Author: ivp
 */

#ifndef IVPPLUGINSTATUS_H_
#define IVPPLUGINSTATUS_H_

#include "../tmx.h"
#include "../IvpMessage.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef cJSON IvpPluginStatusCollection;

typedef struct {
	char *key;
	char *value;
} IvpPluginStatusItem;

int ivpPluginStatus_isStatusMsg(IvpMessage *msg);

IvpMessage *ivpPluginStatus_createMsg(IvpPluginStatusCollection *collection);

IvpPluginStatusCollection *ivpPluginStatus_addStatusItem(IvpPluginStatusCollection *collection, const char *key, const char *value);

int ivpPluginStatus_getItemCount(IvpPluginStatusCollection *collection);
IvpPluginStatusItem *ivpPluginStatus_getItem(IvpPluginStatusCollection *collection, int index);

void ivpPluginStatus_destroyCollection(IvpPluginStatusCollection *collection);
void ivpPluginStatus_destroyItem(IvpPluginStatusItem *item);

#ifdef __cplusplus
}
#endif

#endif /* IVPPLUGINSTATUS_H_ */
