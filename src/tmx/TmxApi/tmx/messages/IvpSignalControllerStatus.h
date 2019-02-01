#ifndef IVPSCS_H_
#define IVPSCS_H_

#include "../tmx.h"
#include "../IvpMessage.h"
#include "../apimessages/IvpMessageType.h"
#include <stdio.h>
#include <stdint.h>

#define IVPMSG_TYPE_SIGCONT "SIGCONT"

#ifdef __cplusplus
extern "C"
{
#endif

int ivpSigCont_isSigContMsg(IvpMessage *msg);

IvpMessage *ivpSigCont_createMsg(int action);

int ivpSigCont_getIvpSignalControllerAction(IvpMessage *msg);


#ifdef __cplusplus
}
#endif

#endif /* IVPSCS_H_ */
