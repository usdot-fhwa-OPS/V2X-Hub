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
#include <ITIScodesAndText.h>

#if SAEJ2735_SPEC < 63
typedef TravelerInformation::TravelerInformation__dataFrames::TravelerInformation__dataFrames__List::TravelerInformation__dataFrames__Member TiDataFrame;
typedef ITIScodesAndText::ITIScodesAndText__List::ITIScodesAndText__Member ItisMember;
typedef ITIScodesAndText::ITIScodesAndText__List ItisList;
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
