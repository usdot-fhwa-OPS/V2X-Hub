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
#if SAEJ2735_SPEC < 63
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
#if SAEJ2735_SPEC > 2020
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
#include <TestMessage11.h>
#include <TestMessage12.h>
#include <TestMessage13.h>
#include <TestMessage14.h>
#include <TestMessage15.h>
#endif

static asn_TYPE_descriptor_t *getTypeDescriptorFromMsgId(e_DSRCmsgID msgId);
static const char *getMessageSubTypeFromMsgId(e_DSRCmsgID msgId);
static e_DSRCmsgID getMsgIdFromMessageSubType(const char *subtype);

asn_TYPE_descriptor_t *getTypeDescriptorFromMsgId(e_DSRCmsgID msgId)
{
	switch(msgId)
	{
//		case DSRCmsgID_alaCarteMessage: return &asn_DEF_AlaCarte;
		case DSRCmsgID_basicSafetyMessage: return &asn_DEF_BasicSafetyMessage;
		#if SAEJ2735_SPEC < 63
		case DSRCmsgID_basicSafetyMessageVerbose: return &asn_DEF_BasicSafetyMessageVerbose;
		#endif
		case DSRCmsgID_commonSafetyRequest: return &asn_DEF_CommonSafetyRequest;
		case DSRCmsgID_emergencyVehicleAlert: return &asn_DEF_EmergencyVehicleAlert;
		#if SAEJ2735_SPEC < 2024
		case DSRCmsgID_intersectionCollisionAlert: return &asn_DEF_IntersectionCollision;
		#else
		case DSRCmsgID_intersectionCollision: return &asn_DEF_IntersectionCollision;
		#endif
		case DSRCmsgID_mapData: return &asn_DEF_MapData;
		case DSRCmsgID_nmeaCorrections:
		#if SAEJ2735_SPEC < 63
			return &asn_DEF_NMEA_Corrections;
		#else
			return &asn_DEF_NMEAcorrections;
		#endif
		case DSRCmsgID_probeDataManagement: return &asn_DEF_ProbeDataManagement;
		case DSRCmsgID_probeVehicleData: return &asn_DEF_ProbeVehicleData;
		case DSRCmsgID_roadSideAlert: return &asn_DEF_RoadSideAlert;
		case DSRCmsgID_rtcmCorrections:
		#if SAEJ2735_SPEC < 63
			return &asn_DEF_RTCM_Corrections;
		#else
			return &asn_DEF_RTCMcorrections;
		#endif
		case DSRCmsgID_signalPhaseAndTimingMessage: return &asn_DEF_SPAT;
		case DSRCmsgID_signalRequestMessage:
		#if SAEJ2735_SPEC < 63
			return &asn_DEF_SignalRequestMsg;
		#else
			return &asn_DEF_SignalRequestMessage;
		#endif
		case DSRCmsgID_signalStatusMessage: return &asn_DEF_SignalStatusMessage;
		case DSRCmsgID_travelerInformation: return &asn_DEF_TravelerInformation;
		default: break;
	}

	return NULL;
}

const char *getMessageSubTypeFromMsgId(e_DSRCmsgID msgId)
{
	switch(msgId)
	{
//		case DSRCmsgID_alaCarteMessage: return "ACM";
		case DSRCmsgID_basicSafetyMessage: return "BSM";
		#if SAEJ2735_SPEC < 2024
		case DSRCmsgID_basicSafetyMessageVerbose: return "BSMV";
		case DSRCmsgID_intersectionCollisionAlert: return "IC";
		#else
		case DSRCmsgID_intersectionCollision: return "ICA";
		#endif
		case DSRCmsgID_commonSafetyRequest: return "CSR";
		case DSRCmsgID_emergencyVehicleAlert: return "EVA";
		case DSRCmsgID_mapData: return "MAP";
		case DSRCmsgID_nmeaCorrections: return "NMEA";
		case DSRCmsgID_probeDataManagement: return "PDM";
		case DSRCmsgID_probeVehicleData: return "PVD";
		case DSRCmsgID_roadSideAlert: return "RSA";
		case DSRCmsgID_rtcmCorrections: return "RTCM";
		case DSRCmsgID_signalPhaseAndTimingMessage: return "SPAT";
		case DSRCmsgID_signalRequestMessage: return "SRM";
		case DSRCmsgID_signalStatusMessage: return "SSM";
		case DSRCmsgID_travelerInformation: return "TIM";
		case DSRCmsgID_personalSafetyMessage: return "PSM";
		case DSRCmsgID_sensorDataSharingMessage: return "SDSM";
		#if SAEJ2735_SPEC > 2020
		case DSRCmsgID_roadSafetyMessage: return "RSM";
		case DSRCmsgID_roadWeatherMessage: return "RWM";
		case DSRCmsgID_probeDataConfigMessage: return "PDC";
		case DSRCmsgID_probeDataReportMessage: return "PDR";
		case DSRCmsgID_tollAdvertisementMessage: return "TAM";
		case DSRCmsgID_tollUsageMessage: return "TUM";
		case DSRCmsgID_tollUsageAckMessage: return "TUMack";
		case DSRCmsgID_cooperativeControlMessage: return "CCM";
		case DSRCmsgID_maneuverSharingAndCoordinatingMessage: return "MSCM";
		case DSRCmsgID_roadGeometryAndAttributes: return "RGA";
		case DSRCmsgID_personalSafetyMessage2: return "PSM2";
		case DSRCmsgID_trafficSignalPhaseAndTiming: return "TSPaT";
		case DSRCmsgID_signalControlAndPrioritizationRequest: return "SCPR";
		case DSRCmsgID_signalControlAndPrioritizationStatus: return "SCPS";
		case DSRCmsgID_roadUserChargingConfigMessage: return "RUCCM";
		case DSRCmsgID_roadUserChargingReportMessage: return "RUCRM";
		case DSRCmsgID_trafficLightStatusMessage: return "TLSM";
		#endif
		default: break;
	}

	return NULL;
}

const char *getMessageDescriptionFromMsgId(e_DSRCmsgID msgId)
{
	switch(msgId)
	{
//		case DSRCmsgID_alaCarteMessage: return "DSRC AlaCarte Message";
		case DSRCmsgID_basicSafetyMessage: return "DSRC Basic Safety Message";
		#if SAEJ2735_SPEC < 2024
		case DSRCmsgID_basicSafetyMessageVerbose: return "DSRC Basic Safety Message Verbose";
		case DSRCmsgID_intersectionCollisionAlert: return "DSRC Intersection Collision";
		#else
		case DSRCmsgID_intersectionCollision: return "DSRC Intersection Collision";
		#endif
		case DSRCmsgID_commonSafetyRequest: return "DSRC Common Safety Request";
		case DSRCmsgID_emergencyVehicleAlert: return "DSRC Emergency Vehicle Alert";
		case DSRCmsgID_mapData: return "DSRC Map Data";
		case DSRCmsgID_nmeaCorrections: return "DSRC NMEA Corrections";
		case DSRCmsgID_probeDataManagement: return "DSRC Probe Data Management";
		case DSRCmsgID_probeVehicleData: return "DSRC Probe Vehicle Data";
		case DSRCmsgID_roadSideAlert: return "DSRC Road Side Alert";
		case DSRCmsgID_rtcmCorrections: return "DSRC RTCM Corrections";
		case DSRCmsgID_signalPhaseAndTimingMessage: return "DSRC SPAT Message";
		case DSRCmsgID_signalRequestMessage: return "DSRC Signal Request Message";
		case DSRCmsgID_signalStatusMessage: return "DSRC Signal Status Message";
		case DSRCmsgID_travelerInformation: return "DSRC Traveler Information Message";
		case DSRCmsgID_personalSafetyMessage: return "DSRC Personal Safety Message";
		case DSRCmsgID_sensorDataSharingMessage: return "DSRC Sensor Data Sharing Message";
		#if SAEJ2735_SPEC > 2020
		case DSRCmsgID_roadSafetyMessage: return "DSRC Road Safety Message";
		case DSRCmsgID_roadWeatherMessage: return "DSRC Road Weather Message";
		case DSRCmsgID_probeDataConfigMessage: return "DSRC Probe Data Config Message";
		case DSRCmsgID_probeDataReportMessage: return "DSRC Probe Data Report Message";
		case DSRCmsgID_tollAdvertisementMessage: return "DSRC Toll Advertisement Message";
		case DSRCmsgID_tollUsageMessage: return "DSRC Toll Usage Message";
		case DSRCmsgID_tollUsageAckMessage: return "DSRC Toll Usage Ack Message";
		case DSRCmsgID_cooperativeControlMessage: return "DSRC Cooperative Control Message";
		case DSRCmsgID_maneuverSharingAndCoordinatingMessage: return "DSRC Maneuver Sharing And Coordinating Message";
		case DSRCmsgID_roadGeometryAndAttributes: return "DSRC Road Geometry And Attributes";
		case DSRCmsgID_personalSafetyMessage2: return "DSRC Personal Safety Message2";
		case DSRCmsgID_trafficSignalPhaseAndTiming: return "DSRC Traffic Signal Phase And Timing";
		case DSRCmsgID_signalControlAndPrioritizationRequest: return "DSRC Signal Control And Prioritization Request";
		case DSRCmsgID_signalControlAndPrioritizationStatus: return "DSRC Signal Control And Prioritization Status";
		case DSRCmsgID_roadUserChargingConfigMessage: return "DSRC Road User Charging Config Message";
		case DSRCmsgID_roadUserChargingReportMessage: return "DSRC Road User Charging Report Message";
		case DSRCmsgID_trafficLightStatusMessage: return "DSRC Traffic Light Status Message";
		#endif
		default: break;
	}

	return NULL;
}

e_DSRCmsgID getMsgIdFromMessageSubType(const char *subtype)
{
	assert(subtype != NULL);
	if (subtype == NULL)
		#if SAEJ2735_SPEC < 2024
		return DSRCmsgID_reserved;
		#else
		return DSRCmsgID_reservedMessageId_D;
		#endif

	int i;
	for(i = 0; i <= 255; i++)
	{
		if (getMessageSubTypeFromMsgId(i) != NULL)
			if (strcmp(subtype, getMessageSubTypeFromMsgId(i)) == 0)
				return i;
	}

	#if SAEJ2735_SPEC < 2024
	return DSRCmsgID_reserved;
	#else
	return DSRCmsgID_reservedMessageId_D;
	#endif
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

	#if SAEJ2735_SPEC < 2024
	return DSRCmsgID_reserved;
	#else
	return DSRCmsgID_reservedMessageId_D;
	#endif
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

	#if SAEJ2735_SPEC < 2024
	if (msgId == DSRCmsgID_reserved)
	#else
	if (msgId == DSRCmsgID_reservedMessageId_D)
	#endif
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
		#if SAEJ2735_SPEC < 2024
		if (msgId != DSRCmsgID_reserved)
		#else
		if (msgId != DSRCmsgID_reservedMessageId_D)
		#endif
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

	#if SAEJ2735_SPEC < 2024
	if (msgId == DSRCmsgID_reserved)
	#else
	if (msgId == DSRCmsgID_reservedMessageId_D)
	#endif
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

	#if SAEJ2735_SPEC < 2024
	if (msgId == DSRCmsgID_reserved)
	#else
	if (msgId == DSRCmsgID_reservedMessageId_D)
	#endif
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


