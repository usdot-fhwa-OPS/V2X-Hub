/*
 * PmmRequestMessageEnumTypes.h
 *
 *  Created on: Oct 20, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_PMMREQUESTMESSAGEENUMTYPES_H_
#define INCLUDE_PMMREQUESTMESSAGEENUMTYPES_H_

namespace tmx {

namespace messages {

namespace pmmrequestmessage {


enum MobilityNeedTypes
{
	NoSpecialNeeds = 0,
	Wheelchair = 1,
	NeedsSeat = 2
};

enum StatusTypes
{
	New = 0,
	Updated = 1,
	Arrive = 2
};

enum ModeOfTransportTypes
{
	noPreference	= 0,
    transit	= 1,
	taxi = 2,
	rideShare = 3
};



static constexpr const char *MOBILITYNEEDSTYPES_NOSPECIALNEEDS_STRING = "NoSpecialNeeds";
static constexpr const char *MOBILITYNEEDSTYPES_WHEELCHAIR_STRING = "Wheelchair";
static constexpr const char *MOBILITYNEEDSTYPES_NEEDSSEAT_STRING = "NeedsSeat";

} /* End namespace pmmrequestmessage */

} /* End namespace messages */

} /* End namespace tmx */


#endif /* INCLUDE_PMMREQUESTMESSAGEENUMTYPES_H_ */
