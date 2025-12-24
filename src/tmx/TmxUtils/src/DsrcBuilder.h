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

#if SAEJ2735_SPEC >= 2024
#include <ITIS_ITIScodesAndText.h>
#else
#include <ITIScodesAndText.h>
#include <NodeListXY.h>
#include <Count.h>
#endif

#if SAEJ2735_SPEC >= 2024
using TiDataFrame =TravelerDataFrame;
using ITIScodesAndText=ITIS_ITIScodesAndText;
using ITIScodesAndText__Member=ITIS_ITIScodesAndText__Member;
using ItisList=ITIS_ITIScodesAndText_t;
#else
using TiDataFrame=TravelerDataFrame;
using ItisMember=ITIScodesAndText__Member;
using ItisList=ITIScodesAndText;
using NodeList_t=NodeListXY;
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
