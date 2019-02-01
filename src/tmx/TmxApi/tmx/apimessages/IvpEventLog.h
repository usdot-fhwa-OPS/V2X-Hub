/*
 * IvpEventLog.h
 *
 *  Created on: Jul 31, 2014
 *      Author: ivp
 */

#ifndef IVPEVENTLOG_H_
#define IVPEVENTLOG_H_

#include "IvpError.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
	IvpLogLevel level;
	char *description;
} IvpEventLogEntry;

int ivpEventLog_isEventLogMsg(IvpMessage *msg);

IvpMessage *ivpEventLog_createMsg(IvpLogLevel level, const char *description);

IvpEventLogEntry *ivpEventLog_getEventLogEntry(IvpMessage *msg);

void ivpEventLog_destoryEventLogEntry(IvpEventLogEntry *entry);


#ifdef __cplusplus
}
#endif

#endif /* IVPEVENTLOG_H_ */
