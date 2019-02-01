/*
 * MsgFramer.h
 *
 *  Created on: Jul 22, 2014
 *      Author: ivp
 */

#ifndef MSGFRAMER_H_
#define MSGFRAMER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define MSG_FRAMER_MAX_BUF_SIZE 20000

typedef struct {
	char buf[MSG_FRAMER_MAX_BUF_SIZE + 1];
	int bufpos;
	int readpos;
	void (*onMsgFound)(char *);
} MsgFramer;

extern const MsgFramer MSG_FRAMER_INITIALIZER;

char *msgFramer_getBuf(MsgFramer *framer);
int msgFramer_getBufLength(MsgFramer *framer);
void msgFramer_incrementBufPos(MsgFramer *framer, int incLength);
char *msgFramer_getNextMsg(MsgFramer *framer);
char *msgFramer_createFramedMsg(char *msg, int msgLength, int *outMsgLength);


#ifdef __cplusplus
}
#endif

#endif /* MSGFRAMER_H_ */
