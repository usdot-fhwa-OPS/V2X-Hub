/*
 * DsrcBuilder.h
 *
 *  Created on: Sep 25, 2014
 *      Author: ivp
 */

#ifndef DSRCBUILDER_H_
#define DSRCBUILDER_H_

#include <string>
#include <stdexcept>

#include <TravelerInformation.h>
#include "Clock.h"
#include <string>
#include <sstream>
#include <OCTET_STRING.h>

#if SAEJ2735_SPEC == 2024
#include <ITIS_ITIScodesAndText.h>
#else
#include <ITIScodesAndText.h>
#endif

#if SAEJ2735_SPEC < 2016
typedef TravelerInformation::TravelerInformation__dataFrames::TravelerInformation__dataFrames__List::TravelerInformation__dataFrames__Member TiDataFrame;
typedef ITIScodesAndText::ITIScodesAndText__List::ITIScodesAndText__Member ItisMember;
typedef ITIScodesAndText::ITIScodesAndText__List ItisList;
#elif SAEJ2735_SPEC == 2024
typedef TravelerDataFrame TiDataFrame;
typedef ITIS_ITIScodesAndText ITIScodesAndText;
typedef ITIS_ITIScodesAndText__Member ITIScodesAndText__Member;
typedef ITIS_ITIScodesAndText_t ItisList;
// typedef ITIS_ITIScodesAndText__Member__item_PR::ITIS_ITIScodesAndText__Member__item_PR_text ITIScodesAndText__Member__item_PR_text;
// typedef ITIS_ITIScodesAndText__Member__item_PR::ITIS_ITIScodesAndText__Member__item_PR_itis ITIScodesAndText__Member__item_PR_itis;

#else
typedef TravelerDataFrame TiDataFrame;
typedef ITIScodesAndText__Member ItisMember;
typedef ITIScodesAndText ItisList;
#include <NodeListXY.h>
typedef NodeListXY NodeList_t;
#include <Count.h>
#endif

class DsrcBuilder
{
public:
	static void AddTimAdvisory(TiDataFrame *frame, unsigned int speedLimit);
	static void AddItisCode(ItisList *list, long code);
	static void AddItisText(ItisList *list, std::string text);
	static void SetPacketId(TravelerInformation *tim);
	static void SetStartTimeToYesterday(TiDataFrame *frame);
};

#endif /* DSRCBUILDER_H_ */
