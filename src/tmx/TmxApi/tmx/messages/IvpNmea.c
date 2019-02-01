/*
 * IvpNmea.c
 *
 *  Created on: Aug 23, 2014
 *      Author: ivp
 */

#include "IvpNmea.h"
#include <assert.h>
#include <string.h>

const char *getSubType(const char *sentence)
{
	//assert(strstr(sentence, "$GP") == sentence);

	if(strstr(sentence, "$GPGGA") == sentence)
		return "GGA";
	if(strstr(sentence, "$GPGSA") == sentence)
		return "GSA";
	if(strstr(sentence, "$GPRMC") == sentence)
		return "RMC";
	if(strstr(sentence, "$GPGSV") == sentence)
		return "GSV";
	if(strstr(sentence, "$GPVTG") == sentence)
		return "VTG";
	if(strstr(sentence, "$GPVTG") == sentence)
		return "GLL";

	return NULL;
}

int ivpNmea_isNmeaMsg(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return 0;
	return msg->type != NULL && strcmp(msg->type, IVPMSG_TYPE_NMEA) == 0;
}

IvpMessage *ivpNmea_createMsg(const NmeaString *nmeaSentense)
{
	assert(nmeaSentense != NULL);
	if (nmeaSentense == NULL)
		return NULL;

	IvpMessage *results = NULL;

	cJSON *payload = cJSON_CreateString(nmeaSentense);
	assert(payload != NULL);
	if (payload != NULL)
	{
		const char *subtype = getSubType(nmeaSentense);
		//assert(subtype != NULL);
		if (subtype != NULL)
		{
			results = ivpMsg_create(IVPMSG_TYPE_NMEA, subtype, IVP_ENCODING_STRING, IvpMsgFlags_None, payload);
			assert(results != NULL);
		}
	}

	return results;
}

NmeaString *ivpNmea_getNmeaString(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return NULL;

	NmeaString *results = NULL;

	if (msg->payload != NULL && msg->payload->type == cJSON_String)
		results = strdup(msg->payload->valuestring);

	return results;
}

void ivpNmea_freeNmeaString(NmeaString *string)
{
	assert(string != NULL);
	if (string == NULL)
		return;
	free(string);
}

