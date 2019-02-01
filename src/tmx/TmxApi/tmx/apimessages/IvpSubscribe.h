/*
 * IvpSubscribe.h
 *
 *  Created on: Jul 19, 2014
 *      Author: ivp
 */

#ifndef IVPSUBSCRIBE_H_
#define IVPSUBSCRIBE_H_

#include "../tmx.h"
#include "../IvpMessage.h"
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef cJSON IvpMsgFilter;

IvpMessage *ivpSubscribe_createMsg(IvpMsgFilter *filter);

int ivpSubscribe_isSubscribeMsg(IvpMessage *msg);

IvpMsgFilter *ivpSubscribe_addFilterEntry(IvpMsgFilter *filter, const char *type, const char *subtype);
IvpMsgFilter *ivpSubscribe_addFilterEntryWithFlagMask(IvpMsgFilter *filter, const char *type, const char *subtype, IvpMsgFlags flags);
void ivpSubscribe_destroyFilter(IvpMsgFilter *filter);

int ivpSubscribe_getFilterEntryCount(IvpMessage *msg);
void ivpSubscribe_getFilterEntry(IvpMessage *msg, int index, char **typeout, char **subtypeout, IvpMsgFlags *flagsout);


#ifdef __cplusplus
}
#endif

#endif /* IVPSUBSCRIBE_H_ */
