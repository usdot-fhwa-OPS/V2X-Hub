/*
 * BsmConverter.cpp
 *
 *  Created on: Jun 10, 2016
 *      Author: ivp
 */

#include "BsmConverter.h"
#include "PluginLog.h"

namespace tmx {
namespace utils {

void BsmConverter::ToDecodedBsmMessage(BasicSafetyMessage_t &bsm, tmx::messages::DecodedBsmMessage &decoded)
{

	decoded.set_IsLocationValid(false);
	decoded.set_IsElevationValid(false);
	decoded.set_IsSpeedValid(false);
	decoded.set_IsHeadingValid(false);
	decoded.set_IsSteeringWheelAngleValid(false);


	uint32_t uint32Temp;

	memcpy(&uint32Temp,bsm.coreData.id.buf, 4);
	decoded.set_TemporaryId(uint32Temp);

	int32_t latitude = bsm.coreData.lat;
	int32_t longitude = bsm.coreData.Long;

	if (latitude != 900000001 && longitude != 1800000001)
	{
		decoded.set_Latitude((double)latitude/10000000);
		decoded.set_Longitude((double)longitude/10000000);
		decoded.set_IsLocationValid(true);
	}

	int16_t elevation = bsm.coreData.elev;

	if (elevation != -4095)
	{
		decoded.set_Elevation_m((float)elevation/10.0);
		decoded.set_IsElevationValid(true);
	}

	

	// The speed is contained in bits 0-12.  Units are 0.02 meters/sec.
	// A value of 8191 is used when the speed is not known.
	uint16_t speed = bsm.coreData.speed;

	if (speed != 8191)
	{
		// Convert from .02 meters/sec to mps.
		decoded.set_Speed_mps(speed / 50.0);
		decoded.set_IsSpeedValid(true);
	}

	// Heading units are 0.0125 degrees.
	uint16_t heading = bsm.coreData.heading;

	if (heading != 28800)
	{
		decoded.set_Heading(heading / 80.0);
		decoded.set_IsHeadingValid(true);
	}

	// Steering Wheel Angle units are 1.5 degrees.
	int16_t angle = bsm.coreData.angle;

	if (angle != 127)
	{
		decoded.set_SteeringWheelAngle(angle * 1.5);
		decoded.set_IsSteeringWheelAngleValid(true);
	}

}


void BsmConverter::ToBasicSafetyMessage(tmx::messages::DecodedBsmMessage &decoded, BasicSafetyMessage_t &bsm)
{
	memset(&bsm, 0, sizeof(BasicSafetyMessage_t));

	int8_t int8Temp;
	int16_t int16Temp;
	uint16_t uint16Temp;
	uint16_t uint16Temp2;
	uint32_t uint32Temp;
	int32_t int32Temp2;


	//if(iMsgCount>127 || iMsgCount<0)
	//	iMsgCount = 0;

	bsm.coreData.msgCnt = 0;//iMsgCount % 127;

	//iMsgCount++;

	char buf[4];

	uint32Temp = decoded.get_TemporaryId();
	memcpy(buf, &uint32Temp, 4);

	OCTET_STRING_fromBuf(&bsm.coreData.id, buf, 4);

	bsm.coreData.secMark = decoded.get_SecondMark();


	// Latitude and Longitude are expressed in 1/10th integer microdegrees, as a 31 bit value.
	// The value 900000001 indicates that latitude is not available.
	// The value 1800000001 indicates that longitude is not available.

	if (decoded.get_IsLocationValid())
		int32Temp2 = (int32_t)(decoded.get_Latitude() * 10000000.0);
	else
		int32Temp2 = 900000001;

	bsm.coreData.lat = int32Temp2;

	if (decoded.get_IsLocationValid())
		int32Temp2 = (int32_t)(decoded.get_Longitude() * 10000000.0);
	else
		int32Temp2 = 1800000001;

	bsm.coreData.Long = int32Temp2;

	// Elevation is in units of 10 cm steps.
	// max and min values are limited as set below.

	if (decoded.get_IsElevationValid())
	{
		if (decoded.get_Elevation_m() > 6143.9)
			int16Temp = 61439;
		else if (decoded.get_Elevation_m() < -409.5)
			int16Temp = -4095;
		int16Temp = decoded.get_Elevation_m() * 10.0;
	}
	else
		int16Temp = -4095;

	bsm.coreData.elev = int16Temp;

	// Convert from mps to .02 meters/sec.
	if (decoded.get_IsSpeedValid())
		int16Temp = decoded.get_Speed_mps() * 50.0;
	else
		int16Temp = 8191;

	bsm.coreData.speed = int16Temp;


	// Heading units are 0.0125 degrees.
	if (decoded.get_IsHeadingValid())
	{
		if (decoded.get_Heading() > 359.9875)
			uint16Temp2 = 359.9875 * 80;
		else if (decoded.get_Heading() < 0)
			uint16Temp2 = 0;
		uint16Temp2 = decoded.get_Heading() * 80;
	}
	else
		uint16Temp2 = 28800;

	bsm.coreData.heading=uint16Temp2;

	// Steering Wheel Angle units are 1.5 degrees (-126 to 127).
	if (decoded.get_IsSteeringWheelAngleValid())
	{
		if (decoded.get_SteeringWheelAngle() > 189)
			int8Temp = 126;
		else if (decoded.get_SteeringWheelAngle() < -189)
			int8Temp = -126;
		int8Temp = decoded.get_SteeringWheelAngle() * 80;
	}
	else
		int8Temp = 127;

	
	bsm.coreData.angle = int8Temp;

	bsm.coreData.accuracy.semiMajor =0;
	bsm.coreData.accuracy.semiMinor=0;
	bsm.coreData.accuracy.orientation=0;

	bsm.coreData.transmission = TransmissionState_unavailable;

	bsm.coreData.accelSet.Long=0;
	bsm.coreData.accelSet.lat=0;
	bsm.coreData.accelSet.vert=0;
	bsm.coreData.accelSet.yaw=0;

	bsm.coreData.brakes.wheelBrakes.buf = (uint8_t*)malloc(sizeof(uint8_t));
	bsm.coreData.brakes.wheelBrakes.buf[0] = 0;
	bsm.coreData.brakes.wheelBrakes.size=1;
	bsm.coreData.brakes.wheelBrakes.bits_unused =3;

	bsm.coreData.brakes.traction=TractionControlStatus_unavailable;
	bsm.coreData.brakes.abs=AntiLockBrakeStatus_unavailable;
	bsm.coreData.brakes.scs=StabilityControlStatus_unavailable;
	bsm.coreData.brakes.brakeBoost=BrakeBoostApplied_unavailable;
	bsm.coreData.brakes.auxBrakes=AuxiliaryBrakeStatus_unavailable;

}

} /* namespace utils */
} /* namespace tmx */
