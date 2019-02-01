/*
 * IvpMessageType.h
 *
 *  Created on: Aug 8, 2014
 *      Author: ivp
 */

#ifndef IVPMESSAGETYPE_H_
#define IVPMESSAGETYPE_H_

#include "IvpError.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef cJSON IvpMessageTypeCollection;

typedef struct {
	char *type;
	char *subtype;
	char *description;
} IvpMessageTypeEntry;

IvpMessageTypeCollection *ivpMsgType_addEntryToCollection(IvpMessageTypeCollection *collection, const char *type, const char *subtype, const char *description);

IvpError ivpMsgType_validateCollection(IvpMessageTypeCollection *collection);

int ivpMsgType_getEntryCount(IvpMessageTypeCollection *collection);
IvpMessageTypeEntry *ivpMsgType_getEntry(IvpMessageTypeCollection *collection, int index);

void ivpMsgType_destroyCollection(IvpMessageTypeCollection *collection);
void ivpMsgType_destroyEntry(IvpMessageTypeEntry *entry);

#ifdef __cplusplus
}
#endif

#endif /* IVPMESSAGETYPE_H_ */
