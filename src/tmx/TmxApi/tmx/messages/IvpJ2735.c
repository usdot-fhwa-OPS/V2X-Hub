/*
 * IvpJ2735.c
 *
 *  Created on: Aug 7, 2014
 *      Author: ivp
 */

#include "IvpJ2735.h"

#include <assert.h>

//#include <asn_j2735/AlaCarte.h>
#include <BasicSafetyMessage.h>
#if SAEJ2735_SPEC < 2016
#include <BasicSafetyMessageVerbose.h>
#include <NMEA-Corrections.h>
#include <RTCM-Corrections.h>
#include <SignalRequestMsg.h>
#else
#include <NMEAcorrections.h>
#include <RTCMcorrections.h>
#include <SignalRequestMessage.h>
#endif
#include <CommonSafetyRequest.h>
#include <EmergencyVehicleAlert.h>
#include <IntersectionCollision.h>
#include <MapData.h>
#include <ProbeDataManagement.h>
#include <ProbeVehicleData.h>
#include <RoadSideAlert.h>
#include <SPAT.h>
#include <SignalStatusMessage.h>
#include <TravelerInformation.h>
#include <PersonalSafetyMessage.h>
#include <TestMessage00.h>
#include <TestMessage01.h>
#include <TestMessage02.h>
#include <TestMessage03.h>
#include <TestMessage04.h>
#include <TestMessage05.h>
#if SAEJ2735_SPEC >= 2024
#include <RoadSafetyMessage.h>
#include <RoadWeatherMessage.h>
#include <ProbeDataConfigMessage.h>
#include <ProbeDataReportMessage.h>
#include <TollAdvertisementMessage.h>
#include <TollUsageMessage.h>
#include <TollUsageAckMessage.h>
#include <CooperativeControlMessage.h>
#include <SensorDataSharingMessage.h>
#include <ManeuverSharingAndCoordinatingMessage.h>
#include <RoadGeometryAndAttributes.h>
#include <PersonalSafetyMessage2.h>
#include <TrafficSignalPhaseAndTiming.h>
#include <SignalControlAndPrioritizationRequest.h>
#include <SignalControlAndPrioritizationStatus.h>
#include <RoadUserChargingConfigMessage.h>
#include <RoadUserChargingReportMessage.h>
#include <TrafficLightStatusMessage.h>
#include <TestMessage06.h>
#include <TestMessage07.h>
#include <TestMessage08.h>
#include <TestMessage09.h>
#include <TestMessage10.h>
#endif
static asn_TYPE_descriptor_t *getTypeDescriptorFromMsgId(e_DSRCmsgID msgId);
static const char *getMessageSubTypeFromMsgId(e_DSRCmsgID msgId);
static e_DSRCmsgID getMsgIdFromMessageSubType(const char *subtype);

asn_TYPE_descriptor_t *getTypeDescriptorFromMsgId(e_DSRCmsgID msgId)
{
	switch(msgId)
	{
//		case DSRCmsgID_alaCarteMessage: return &asn_DEF_AlaCarte;
		case msgID_basicSafetyMessage: return &asn_DEF_BasicSafetyMessage;
#if SAEJ2735_SPEC < 2016
		case msgID_basicSafetyMessageVerbose: return &asn_DEF_BasicSafetyMessageVerbose;
#endif
		case msgID_commonSafetyRequest: return &asn_DEF_CommonSafetyRequest;
		case msgID_emergencyVehicleAlert: return &asn_DEF_EmergencyVehicleAlert;
		case msgID_intersectionCollisionAlert: return &asn_DEF_IntersectionCollision;
		case msgID_mapData: return &asn_DEF_MapData;
		case msgID_nmeaCorrections:
#if SAEJ2735_SPEC < 2016
			return &asn_DEF_NMEA_Corrections;
#else
			return &asn_DEF_NMEAcorrections;
#endif
		case msgID_probeDataManagement: return &asn_DEF_ProbeDataManagement;
		case msgID_probeVehicleData: return &asn_DEF_ProbeVehicleData;
		case msgID_roadSideAlert: return &asn_DEF_RoadSideAlert;
		case msgID_rtcmCorrections:
#if SAEJ2735_SPEC < 2016
			return &asn_DEF_RTCM_Corrections;
#else
			return &asn_DEF_RTCMcorrections;
#endif
		case msgID_signalPhaseAndTimingMessage: return &asn_DEF_SPAT;
		case msgID_signalRequestMessage:
#if SAEJ2735_SPEC < 2016
			return &asn_DEF_SignalRequestMsg;
#else
			return &asn_DEF_SignalRequestMessage;
#endif
		case msgID_signalStatusMessage: return &asn_DEF_SignalStatusMessage;
		case msgID_travelerInformation: return &asn_DEF_TravelerInformation;
		default: break;
	}

	return NULL;
}

const char *getMessageSubTypeFromMsgId(e_DSRCmsgID msgId)
{
	switch(msgId)
	{
//		case msgID_alaCarteMessage: return "ACM";
		case msgID_basicSafetyMessage: return "BSM";
		case msgID_basicSafetyMessageVerbose: return "BSMV";
		case msgID_commonSafetyRequest: return "CSR";
		case msgID_emergencyVehicleAlert: return "EVA";
		case msgID_intersectionCollisionAlert: return "IC"; // I think this can be changed to intersectionCollision
		case msgID_mapData: return "MAP";
		case msgID_nmeaCorrections: return "NMEA";
		case msgID_probeDataManagement: return "PDM";
		case msgID_probeVehicleData: return "PVD";
		case msgID_roadSideAlert: return "RSA";
		case msgID_rtcmCorrections: return "RTCM";
		case msgID_signalPhaseAndTimingMessage: return "SPAT";
		case msgID_signalRequestMessage: return "SRM";
		case msgID_signalStatusMessage: return "SSM";
		case msgID_travelerInformation: return "TIM";
		case msgID_personalSafetyMessage: return "PSM";
		case msgID_sensorDataSharingMessage: return "SDSM";
		case msgID_roadSafetyMessage: return "RSM";
		case msgID_roadWeatherMessage: return "RWM";
		case msgID_probeDataConfigMessage: return "PDC";
		case msgID_probeDataReportMessage: return "PDR";
		case msgID_tollAdvertisementMessage: return "TAM";
		case msgID_tollUsageMessage: return "TUM";
		case msgID_tollUsageAckMessage: return "TUMack";
		case msgID_cooperativeControlMessage: return "CCM";
		case msgID_maneuverSharingAndCoordinatingMessage: return "MSCM";
		case msgID_roadGeometryAndAttributes: return "RGA";
		case msgID_personalSafetyMessage2: return "PSM2";
		case msgID_trafficSignalPhaseAndTiming: return "TSPaT";
		case msgID_signalControlAndPrioritizationRequest: return "SCPR";
		case msgID_signalControlAndPrioritizationStatus: return "SCPS";
		case msgID_roadUserChargingConfigMessage: return "RUCCM";
		case msgID_roadUserChargingReportMessage: return "RUCRM";
		case msgID_trafficLightStatusMessage: return "TLSM";

		default: break;
	}

	return NULL;
}

const char *getMessageDescriptionFromMsgId(e_DSRCmsgID msgId)
{
	switch(msgId)
	{
//		case msgID_alaCarteMessage: return "DSRC AlaCarte Message";
		case msgID_basicSafetyMessage: return "DSRC Basic Safety Message";
		case msgID_basicSafetyMessageVerbose: return "DSRC Basic Safety Message Verbose";
		case msgID_commonSafetyRequest: return "DSRC Common Safety Request";
		case msgID_emergencyVehicleAlert: return "DSRC Emergency Vehicle Alert";
		case msgID_intersectionCollisionAlert: return "DSRC Intersection Collision";
		case msgID_mapData: return "DSRC Map Data";
		case msgID_nmeaCorrections: return "DSRC NMEA Corrections";
		case msgID_probeDataManagement: return "DSRC Probe Data Management";
		case msgID_probeVehicleData: return "DSRC Probe Vehicle Data";
		case msgID_roadSideAlert: return "DSRC Road Side Alert";
		case msgID_rtcmCorrections: return "DSRC RTCM Corrections";
		case msgID_signalPhaseAndTimingMessage: return "DSRC SPAT Message";
		case msgID_signalRequestMessage: return "DSRC Signal Request Message";
		case msgID_signalStatusMessage: return "DSRC Signal Status Message";
		case msgID_travelerInformation: return "DSRC Traveler Information Message";
		case msgID_sensorDataSharingMessage: return "DSRC Sensor Data Sharing Message";
		case msgID_roadSafetyMessage: return "DSRC Road Safety Message";
		case msgID_roadWeatherMessage: return "DSRC Road Weather Message";
		case msgID_probeDataConfigMessage: return "DSRC Probe Data Config Message";
		case msgID_probeDataReportMessage: return "DSRC Probe Data Report Message";
		case msgID_tollAdvertisementMessage: return "DSRC Toll Advertisement Message";
		case msgID_tollUsageMessage: return "DSRC Toll Usage Message";
		case msgID_tollUsageAckMessage: return "DSRC Toll Usage Ack Message";
		case msgID_cooperativeControlMessage: return "DSRC Cooperative Control Message";
		case msgID_maneuverSharingAndCoordinatingMessage: return "DSRC Maneuver Sharing And Coordinating Message";
		case msgID_roadGeometryAndAttributes: return "DSRC Road Geometry And Attributes";
		case msgID_personalSafetyMessage2: return "DSRC Personal Safety Message2";
		case msgID_trafficSignalPhaseAndTiming: return "DSRC Traffic Signal Phase And Timing";
		case msgID_signalControlAndPrioritizationRequest: return "DSRC Signal Control And Prioritization Request";
		case msgID_signalControlAndPrioritizationStatus: return "DSRC Signal Control And Prioritization Status";
		case msgID_roadUserChargingConfigMessage: return "DSRC Road User Charging Config Message";
		case msgID_roadUserChargingReportMessage: return "DSRC Road User Charging Report Message";
		case msgID_trafficLightStatusMessage: return "DSRC Traffic Light Status Message";
		default: break;
	}

	return NULL;
}

e_DSRCmsgID getMsgIdFromMessageSubType(const char *subtype)
{
	assert(subtype != NULL);
	if (subtype == NULL)
		return msgID_reserved;

	int i;
	for(i = 0; i <= 255; i++)
	{
		if (getMessageSubTypeFromMsgId(i) != NULL)
			if (strcmp(subtype, getMessageSubTypeFromMsgId(i)) == 0)
				return i;
	}

	return msgID_reserved;
}

e_DSRCmsgID getMsgIdFromRaw(uint8_t *msg, unsigned int msgLength)
{
	u_int16_t i = 2;
	while(i + 1 < msgLength)
	{
		if(msg[i] == 128 && msg[i + 1] == 1)
		   break;
		else
		   i++;
	}

	if (i != 0 && i + 2 < msgLength)
	{
		return msg[i + 2];
	}

	return msgID_reserved;
}

int ivpJ2735_isJ2735Msg(IvpMessage *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return 0;
	return msg->type != NULL && strcmp(msg->type, IVPMSG_TYPE_J2735) == 0;
}

IvpMessageTypeCollection *ivpJ2735_addMsgTypeToCollection(IvpMessageTypeCollection *collection, e_DSRCmsgID msgId)
{
	if (getTypeDescriptorFromMsgId(msgId) == NULL)
		return collection;

	collection = ivpMsgType_addEntryToCollection(collection, IVPMSG_TYPE_J2735, getMessageSubTypeFromMsgId(msgId), getMessageDescriptionFromMsgId(msgId));
	return collection;
}

IvpMessage *ivpJ2735_createMsg(void *msgStructure, e_DSRCmsgID msgId, IvpMsgFlags flags)
{
	asn_TYPE_descriptor_t *typeDescriptor = getTypeDescriptorFromMsgId(msgId);

	assert(msgStructure != NULL);
	assert(typeDescriptor != NULL);
	if (msgStructure == NULL || typeDescriptor == NULL)
		return NULL;

	char buf[4000];

	asn_enc_rval_t encResults = der_encode_to_buffer(typeDescriptor, msgStructure, &buf[2000], 2000);
	if (encResults.encoded == 0)
		return NULL;

	int i;
	for(i = 0; i < encResults.encoded; i++)
	{
		snprintf(&buf[i*2], 3, "%02x", (uint8_t)buf[i + 2000]);
	}

	IvpMessage *results = NULL;

	cJSON *payload = cJSON_CreateString(buf);
	assert(payload != NULL);
	if (payload != NULL)
	{
		const char *subtype = getMessageSubTypeFromMsgId(msgId);
		assert(subtype != NULL);
		if (subtype != NULL)
		{
			results = ivpMsg_create(IVPMSG_TYPE_J2735, subtype, IVP_ENCODING_ASN1_BER, flags, payload);
			assert(results != NULL);
		}
	}
	cJSON_Delete(payload);
	return results;
}



IvpJ2735Msg *ivpJ2735_getJ2735Msg(IvpMessage *msg)
{
	assert(msg != NULL);
	assert(ivpJ2735_isJ2735Msg(msg));
	assert(msg->subtype != NULL);
	if (msg == NULL || !ivpJ2735_isJ2735Msg(msg) || msg->subtype == NULL)
		return NULL;

	//TODO some of these checks (including subtype from above) can be moved into isJ2735Msg()
	if (msg->payload == NULL)
		return NULL;

	assert(msg->payload->type == cJSON_String);
	if (msg->payload->type != cJSON_String)
		return NULL;

	//vvv the actual working part of the code vvv
	e_DSRCmsgID msgId = getMsgIdFromMessageSubType(msg->subtype);

	if (msgId == msgID_reserved)
		return NULL;

	asn_TYPE_descriptor_t *typeDescriptor = getTypeDescriptorFromMsgId(msgId);
	assert(typeDescriptor != NULL);
	if (typeDescriptor == NULL)
		return NULL;

	uint8_t buf[2000];
	int payloadLength = strlen(msg->payload->valuestring)/2;
	if (payloadLength > sizeof(buf))
		return NULL;

	int i;
	for(i = 0; i < payloadLength; i++)
		sscanf(msg->payload->valuestring + (2 * i), "%2hhx", &buf[i]);

	void *msgStucture = NULL;
	asn_dec_rval_t rval = ber_decode(NULL, typeDescriptor, (void **)&msgStucture, buf, payloadLength);
	if (rval.code == RC_OK && msgStucture)
	{
		IvpJ2735Msg *results = calloc(1, sizeof(IvpJ2735Msg));
		if (results != NULL)
		{
			results->msgId = msgId;
			results->msgStructure = msgStucture;
			return results;
		}

		ASN_STRUCT_FREE(*typeDescriptor, msgStucture);
	}

	return NULL;
}

void ivpJ2735_fprint(FILE *stream, IvpJ2735Msg *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return;

	asn_TYPE_descriptor_t *typeDescriptor = getTypeDescriptorFromMsgId(msg->msgId);
	assert(typeDescriptor != NULL);
	assert(msg->msgStructure != NULL);
	if (typeDescriptor == NULL || msg->msgStructure == NULL)
		return;

	asn_fprint(stream, typeDescriptor, msg->msgStructure);
}

void ivpJ2735_destroyJ2735Msg(IvpJ2735Msg *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return;

	asn_TYPE_descriptor_t *typeDescriptor = getTypeDescriptorFromMsgId(msg->msgId);
	assert(typeDescriptor != NULL);
	if (typeDescriptor == NULL)
		return;

	if (msg->msgStructure != NULL)
		ASN_STRUCT_FREE(*typeDescriptor, msg->msgStructure);

	free(msg);
}

IvpMessage *ivpJ2735_createMsgFromEncoded(uint8_t *msg, unsigned int msgLength, IvpMsgFlags flags)
{
	char buf[4000];

	int i;
	for(i = 0; i < msgLength; i++)
	{
		snprintf(&buf[i*2], 3, "%02x", msg[i]);
	}

	IvpMessage *results = NULL;

	cJSON *payload = cJSON_CreateString(buf);
	assert(payload != NULL);
	if (payload != NULL)
	{
		e_DSRCmsgID msgId = getMsgIdFromRaw(msg, msgLength);
		if (msgId != msgID_reserved)
		{
			const char *subtype = getMessageSubTypeFromMsgId(msgId);
			assert(subtype != NULL);
			if (subtype != NULL)
			{
				results = ivpMsg_create(IVPMSG_TYPE_J2735, subtype, IVP_ENCODING_ASN1_BER, flags, payload);
				assert(results != NULL);
			}
		}
	}
	cJSON_Delete(payload);
	return results;
}

IvpMessage *ivpJ2735_createMsgFromEncodedwType(uint8_t *msg, unsigned int msgLength, IvpMsgFlags flags, const char * msgType)
{
	char buf[4000];

	int i;
	for(i = 0; i < msgLength; i++)
	{
		snprintf(&buf[i*2], 3, "%02x", msg[i]);
	}

	IvpMessage *results = NULL;

	cJSON *payload = cJSON_CreateString(buf);
	assert(payload != NULL);
	if (payload != NULL)
	{
		results = ivpMsg_create(IVPMSG_TYPE_J2735, msgType, IVP_ENCODING_ASN1_BER, flags, payload);
		assert(results != NULL);
	}
	cJSON_Delete(payload);
	return results;
}


IvpJ2735EncodedMsg *ivpJ2735_getJ2735EncodedMsg(IvpMessage *msg)
{
	assert(msg != NULL);
	assert(ivpJ2735_isJ2735Msg(msg));
	assert(msg->subtype != NULL);
	if (msg == NULL || !ivpJ2735_isJ2735Msg(msg) || msg->subtype == NULL)
		return NULL;

	//TODO some of these checks (including subtype from above) can be moved into isJ2735Msg()
	if (msg->payload == NULL)
		return NULL;

	assert(msg->payload->type == cJSON_String);
	if (msg->payload->type != cJSON_String)
		return NULL;

	//vvv the actual working part of the code vvv
	e_DSRCmsgID msgId = getMsgIdFromMessageSubType(msg->subtype);

	if (msgId == msgID_reserved)
		return NULL;

	int payloadLength = strlen(msg->payload->valuestring)/2;

	IvpJ2735EncodedMsg *results = calloc(1, sizeof(IvpJ2735EncodedMsg));
	if (results != NULL)
	{
		results->msgLength = payloadLength;

		results->msg = calloc(payloadLength, sizeof(uint8_t));
		if (results->msg != NULL)
		{
			int i;
			for(i = 0; i < payloadLength; i++)
				sscanf(msg->payload->valuestring + (2 * i), "%2hhx", &results->msg[i]);

			results->msgId = msgId;

			assert(getMsgIdFromRaw(results->msg, results->msgLength) == msgId);
			if (getMsgIdFromRaw(results->msg, results->msgLength) == msgId)
				return results;
		}
	}

	ivpJ2735_destroyJ2735EncodedMsg(results);

	return NULL;
}

void ivpJ2735_destroyJ2735EncodedMsg(IvpJ2735EncodedMsg *msg)
{
	assert(msg != NULL);
	if (msg == NULL)
		return;

	if (msg->msg != NULL)
		free(msg->msg);

	free(msg);
}


IvpJ2735Msg *ivpJ2735_decode(uint8_t *msg, unsigned int msgLength)
{
	assert(msg != NULL);

	//vvv the actual working part of the code vvv
	e_DSRCmsgID msgId = getMsgIdFromRaw(msg, msgLength);

	if (msgId == msgID_reserved)
		return NULL;

	asn_TYPE_descriptor_t *typeDescriptor = getTypeDescriptorFromMsgId(msgId);
	assert(typeDescriptor != NULL);
	if (typeDescriptor == NULL)
		return NULL;

	void *msgStucture = NULL;
	asn_dec_rval_t rval = ber_decode(NULL, typeDescriptor, (void **)&msgStucture, msg, msgLength);
	if (rval.code == RC_OK && msgStucture)
	{
		IvpJ2735Msg *results = calloc(1, sizeof(IvpJ2735Msg));
		if (results != NULL)
		{
			results->msgId = msgId;
			results->msgStructure = msgStucture;
			return results;
		}

		ASN_STRUCT_FREE(*typeDescriptor, msgStucture);
	}

	return NULL;
}


