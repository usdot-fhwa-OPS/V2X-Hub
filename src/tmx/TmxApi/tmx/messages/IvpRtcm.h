/*
 * IvpRtcm.h
 *
 *  Created on: Aug 23, 2014
 *      Author: ivp
 */

#ifndef IVPRTCM_H_
#define IVPRTCM_H_

#include "../tmx.h"
#include "../IvpMessage.h"
#include "../apimessages/IvpMessageType.h"
#include <stdio.h>
#include <stdint.h>

#define IVPMSG_TYPE_RTCM "RTCM"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
	IvpRtcmVersion_Unknown = 0,
	IvpRtcmVersion_23,
	IvpRtcmVersion_30
} IvpRtcmVersion;

typedef struct {
	IvpRtcmVersion version;
	unsigned int dataLength;
	uint8_t *data;
} IvpRtcmData;

int ivpRtcm_isRtcmMsg(IvpMessage *msg);

IvpMessage *ivpRtcm_createMsg(IvpRtcmVersion version, const uint8_t *data, unsigned int length);

IvpRtcmData *ivpRtcm_getRtcmData(IvpMessage *msg);

void ivpRtcm_destroyRtcmData(IvpRtcmData *data);

#ifdef __cplusplus
}
#endif

#endif /* IVPRTCM_H_ */
