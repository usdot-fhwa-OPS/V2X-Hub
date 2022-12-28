/*
 * DsrcBuilder.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: ivp
 */

#include <string>
#include <sstream>
#include <OCTET_STRING.h>
#include <ITIScodesAndText.h>

#include "DsrcBuilder.h"
#include "Clock.h"

using namespace tmx::utils;

void DsrcBuilder::AddCurveSpeedAdvisory(TiDataFrame *frame, unsigned int speedLimit)
{
	frame->content.present = TravelerDataFrame__content_PR_advisory;

	std::stringstream speedText;
	speedText << speedLimit << " MPH";




	AddItisCode(&frame->content.choice.advisory, 27); // "warning advice"
	AddItisText(&frame->content.choice.advisory, "curve ahead");
	AddItisCode(&frame->content.choice.advisory, 2564); // "speed restriction"
	AddItisText(&frame->content.choice.advisory, std::to_string(speedLimit));
	AddItisCode(&frame->content.choice.advisory, 8720); // "MPH"
}

void DsrcBuilder::AddItisCode(ITIScodesAndText *advisory, long code)
{
	ITIScodesAndText__Member* member = (ITIScodesAndText__Member*)malloc(sizeof(ITIScodesAndText__Member));
	member->item.present = ITIScodesAndText__Memberitem_PR_itis;
	member->item.choice.itis = code;
	ASN_SEQUENCE_ADD(&advisory->list, member);
}

void DsrcBuilder::AddItisText(ITIScodesAndText *advisory, std::string text)
{
	int textLength = text.length();

	ITIScodesAndText__Member* member = (ITIScodesAndText__Member*)malloc(sizeof(ITIScodesAndText__Member));
	member->item.present = ITIScodesAndText__Memberitem_PR_text;

	member->item.choice.text.buf = NULL;
	OCTET_STRING_fromString(&(member->item.choice.text), text.c_str());


	ASN_SEQUENCE_ADD(&advisory->list, member);
}

void DsrcBuilder::SetPacketId(TravelerInformation *tim)
{
	// Allocate memory if the packet ID has never been set before.
	if (tim->packetID == NULL)
	{
		tim->packetID = (UniqueMSGID_t *)calloc(1, sizeof(UniqueMSGID_t));
		tim->packetID->buf = (uint8_t *)calloc(9, sizeof(uint8_t));
		tim->packetID->size = 9 * sizeof(uint8_t);
	}

	// Recommended packet ID is Agency ID in the first byte and Publish time of packet (MinuteOfTheYear) in bytes 2-4.
	// Don't know what Agency ID to use.  Specify 0 for now.

	uint32_t minuteOfYear = Clock::GetMinuteOfYear();

	tim->packetID->buf[0] = 0x00;
	tim->packetID->buf[1] = (minuteOfYear & 0xFF0000) >> 16;
	tim->packetID->buf[2] = (minuteOfYear & 0x00FF00) >> 8;
	tim->packetID->buf[3] = minuteOfYear & 0x0000FF;
	tim->packetID->buf[4] = 0x00;
	tim->packetID->buf[5] = 0x00;
	tim->packetID->buf[6] = 0x00;
	tim->packetID->buf[7] = 0x00;
	tim->packetID->buf[8] = 0x00;
}

void DsrcBuilder::SetStartTimeToYesterday(TiDataFrame *frame)
{
	// Set the start time (minutes of the year) to the start of yesterday.
	int dayOfYear = Clock::GetDayOfYear() - 1;
	if (dayOfYear < 0)
		dayOfYear = 364;
	frame->startTime = dayOfYear * 24 * 60;
}
