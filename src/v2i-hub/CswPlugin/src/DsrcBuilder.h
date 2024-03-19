/*
 * DsrcBuilder.h
 *
 *  Created on: Sep 25, 2014
 *      Author: ivp
 */

#ifndef DSRCBUILDER_H_
#define DSRCBUILDER_H_

#include <string>
#include <sstream>
#include <stdexcept>
#include <OCTET_STRING.h>

#include <TravelerInformation.h>
#if SAEJ2735_SPEC < 2024
#include <ITIScodesAndText.h>
#else
#include <ITIS_ITIScodesAndText.h>
#endif

#include "Clock.h"

#if SAEJ2735_SPEC < 63
typedef TravelerInformation::TravelerInformation__dataFrames::TravelerInformation__dataFrames__List::TravelerInformation__dataFrames__Member TiDataFrame;
typedef ITIScodesAndText::ITIScodesAndText__List::ITIScodesAndText__Member ItisMember;
typedef ITIScodesAndText::ITIScodesAndText__List ItisList;
#elif SAEJ2735_SPEC < 2024
typedef TravelerDataFrame TiDataFrame;
typedef ITIScodesAndText__Member ItisMember;
typedef ITIScodesAndText ItisList;
#else
typedef TravelerDataFrame TiDataFrame;
typedef ITIS_ITIScodesAndText__Member ItisMember;
typedef ITIS_ITIScodesAndText ItisList;
#endif

#if SAEJ2735_SPEC > 63
#include <NodeListXY.h>
typedef NodeListXY NodeList_t;
#include <Count.h>
#endif

class DsrcBuilder
{
public:
	static void AddCurveSpeedAdvisory(TiDataFrame *frame, unsigned int speedLimit);
	static void AddItisCode(ItisList *list, long code);
	static void AddItisText(ItisList *list, std::string text);
	static void SetPacketId(TravelerInformation *tim);
	static void SetStartTimeToYesterday(TiDataFrame *frame);
};

#endif /* DSRCBUILDER_H_ */
