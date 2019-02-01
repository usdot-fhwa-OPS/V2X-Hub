/*
 * IvpNmea.h
 *
 *  Created on: Aug 23, 2014
 *      Author: ivp
 */

#ifndef IVPNMEA_H_
#define IVPNMEA_H_


#include "../tmx.h"
#include "../IvpMessage.h"
#include "../apimessages/IvpMessageType.h"
#include <stdio.h>

#define IVPMSG_TYPE_NMEA "NMEA"

#ifdef __cplusplus
extern "C"
{
#endif

typedef char NmeaString;

int ivpNmea_isNmeaMsg(IvpMessage *msg);

IvpMessage *ivpNmea_createMsg(const NmeaString *nmeaSentense);

NmeaString *ivpNmea_getNmeaString(IvpMessage *msg);

void ivpNmea_freeNmeaString(NmeaString *string);

#ifdef __cplusplus
}
#endif

#endif /* IVPNMEA_H_ */
