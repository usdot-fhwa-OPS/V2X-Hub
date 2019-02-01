/*
 * MsgFramer.c
 *
 *  Created on: Jul 22, 2014
 *      Author: ivp
 */

#include "MsgFramer.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const MsgFramer MSG_FRAMER_INITIALIZER = { .bufpos = 0, .readpos = 0, .onMsgFound = NULL };

char *msgFramer_getBuf(MsgFramer *framer)
{
	return framer->buf + framer->bufpos;
}

int msgFramer_getBufLength(MsgFramer *framer)
{
	return MSG_FRAMER_MAX_BUF_SIZE - framer->bufpos;
}

void msgFramer_incrementBufPos(MsgFramer *framer, int incLength)
{
	framer->bufpos += incLength;
	assert(framer->bufpos <= MSG_FRAMER_MAX_BUF_SIZE);
	if (framer->bufpos > MSG_FRAMER_MAX_BUF_SIZE)
	{
		framer->bufpos = 0;
		framer->readpos = 0;
		return;
	}
	framer->buf[framer->bufpos] = '\0';
}


char *msgFramer_getNextMsg(MsgFramer *framer)
{
	char *results = NULL;

	if (framer->readpos != 0)
	{
		int shift = framer->readpos;
		int i;
		for(i = 0; i < framer->bufpos + 1 - shift; i++)
		{
			framer->buf[i] = framer->buf[i + shift];
		}
		//memcpy(framer->buf, framer->buf + shift, framer->bufpos + 1 - shift);
		framer->bufpos -= shift;
		framer->readpos = 0;
	}

	char *msgStart = strstr(framer->buf, "\x02");
	if (msgStart == NULL)
	{
		framer->bufpos = 0;
	}
	else
	{
		if(msgStart != framer->buf)
		{
			int shift = (msgStart - framer->buf);
			//memcpy(framer->buf, framer->buf + shift, framer->bufpos + 1 - shift);
			int i;
			for(i = 0; i < framer->bufpos + 1 - shift; i++)
			{
				framer->buf[i] = framer->buf[i + shift];
			}
			framer->bufpos -= shift;
		}

		char *msgEnd = strstr(framer->buf, "\x03");
		if(msgEnd != NULL)
		{
			*msgEnd = '\0';

			results = framer->buf + 1;
			framer->readpos = (msgEnd - framer->buf) + 1;
		}
	}

	if (framer->bufpos == MSG_FRAMER_MAX_BUF_SIZE && results == NULL)
	{
		framer->bufpos = 0;
		framer->readpos = 0;
	}

	return results;
}

char *msgFramer_createFramedMsg(char *msg, int msgLength, int *outMsgLength)
{
	assert(msg != NULL);
	assert(strchr(msg, 0x02) == NULL);
	assert(strchr(msg, 0x03) == NULL);

	char *framedMsg = malloc(msgLength + 2);
	*outMsgLength = 0;
	if (framedMsg)
	{
		framedMsg[0] = 0x02;
		memcpy(framedMsg + 1, msg, msgLength);
		framedMsg[msgLength + 1] = 0x03;

		*outMsgLength = msgLength + 2;
	}
	return framedMsg;
}
