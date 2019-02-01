#ifndef IVPDMSC_H_
#define IVPDMSC_H_

#include "../tmx.h"
#include "../IvpMessage.h"
#include "../apimessages/IvpMessageType.h"
#include <stdio.h>
#include <stdint.h>

#define IVPMSG_TYPE_DMSCONT "DMSCONT"

#define DMS_MSG_REDUCE_SPEED 1
#define DMS_MSG_CURVE_AHEAD 2

#ifdef __cplusplus
extern "C"
{
#endif

int ivpDmsCont_isSigDmsMsg(IvpMessage *msg);

IvpMessage *ivpDmsCont_createMsg(int dmsMsgId);

int ivpDmsCont_getIvpDmsMsgId(IvpMessage *msg);


#ifdef __cplusplus
}
#endif

#endif /* IVPDMSC_H_ */
