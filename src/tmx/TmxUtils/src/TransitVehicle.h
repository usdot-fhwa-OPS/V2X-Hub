/*
 * TransitVehicle.h
 *
 *  Created on: Mar 17, 2016
 *      Author: Svetlana Jacoby
 */

#ifndef SRC_TRANSITVEHICLE_H_
#define SRC_TRANSITVEHICLE_H_

#include <string>
#include <ApplicationMessage.h>

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::messages::appmessage;

namespace tmx {
namespace utils {

enum TransitVehicleLandingZnPos
{
	TV_IN_RDWAY_ZONES_NONE     = 0x00,
	TV_IN_RDWAY_ZONES_BACK     = 0x01,
	TV_IN_RDWAY_ZONES_FRONT    = 0x02,
	TV_IN_RDWAY_ZONES_BOTH     = 0x03
};

enum TransitVehicleState
{
	TransitVehicleInvalid     = 0,
	TransitVehicleEnteredArea = 1,
	TransitVehicleInArea      = 2,
	TransitVehicleLanding     = 3,
	TransitVehicleArrived     = 4,
	TransitVehicleLeaving     = 5,
	TransitVehicleExitedArea  = 6
};

class TransitVehicle
{
public:
	TransitVehicle(TransitVehicleState bState=TransitVehicleInvalid, uint64_t tStamp=0, string bName="");
	TransitVehicle(EventCodeTypes ev, uint64_t tStamp=0, string bName="");

	virtual ~TransitVehicle();

	TransitVehicleState getState() { return _state; }
	uint64_t getTimestamp() { return _timestamp; }
	string getRawName() { return _rawName; }
	string getRoute() { return _route; }
	string getNumber() { return _number; }
	int8_t getId() { return _id; }

	TransitVehicleState eventToState(EventCodeTypes ev)
	{
		TransitVehicleState bState;

		switch (ev)
		{
		case EnteredArea:
			bState = TransitVehicleEnteredArea;
			break;
		case Landing:
			bState = TransitVehicleLanding;
			break;
		case Arrived:
			bState = TransitVehicleArrived;
			break;
		case Leaving:
			bState = TransitVehicleLeaving;
			break;
		case ExitedArea:
			bState = TransitVehicleExitedArea;
			break;
		default:
			bState = TransitVehicleInvalid;
			break;
		}
		return bState;
	}

private:
	void construct();
	TransitVehicleState _state;
	uint64_t _timestamp;
	string _rawName;
	int8_t _id = -1;
	string _route = "";
	string _number = "";
};

}} // namespace tmx::utils

#endif /* SRC_TRANSITVEHICLE_H_ */
