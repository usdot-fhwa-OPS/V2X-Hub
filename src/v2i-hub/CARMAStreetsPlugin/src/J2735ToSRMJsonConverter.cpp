#include "J2735ToSRMJsonConverter.h"

namespace CARMAStreetsPlugin
{
    J2735ToSRMJsonConverter::J2735ToSRMJsonConverter()
    {
    }

    J2735ToSRMJsonConverter::~J2735ToSRMJsonConverter()
    {
    }

    void J2735ToSRMJsonConverter::toSRMJson(Json::Value &json, tmx::messages::SrmMessage *srm)
    {
        auto srm_ptr = srm->get_j2735_data();
        json["MsgType"] = MsgType;

        /***
         * Request data for one or more signalized intersections that support SRM dialogs.
         */
        Json::Value request;
        request["msgCount"] = 1;
        request["minuteOfYear"] = 345239;
        request["msOfMinute"] = 54000;
        request["regionalID"] = 0;
        request["intersectionID"] = srm_ptr->requests->list.array[0]->request.id.id;
        request["priorityRequestType"] = srm_ptr->requests->list.array[0]->request.requestID;
        request["basicVehicleRole"] = srm_ptr->requestor.type->role;

        Json::Value inBoundLane;
        inBoundLane["LaneID"] = srm_ptr->requests->list.array[0]->request.inBoundLane.choice.lane;
        inBoundLane["ApproachID"] = srm_ptr->requests->list.array[0]->request.inBoundLane.choice.approach;
        request["inBoundLane"] = inBoundLane;

        Json::Value expectedTimeOfArrival;
        // Minute of the year
        expectedTimeOfArrival["ETA_Minute"] = srm_ptr->requests->list.array[0]->minute;
        expectedTimeOfArrival["ETA_Second"] = srm_ptr->requests->list.array[0]->second;
        /**
         * The duration value is used to provide a short interval that extends
         * the ETA so that the requesting vehicle can arrive at the point of service with
         * uncertainty or with some desired duration of service. This concept can be used
         * to avoid needing to frequently update the request. The requester must update
         * the ETA and duration values if the period if services extends beyond the duration
         * time. It should be assumed that if the vehicle does not clear the intersection
         * when the duration is reached, the request will be cancelled and the intersection will
         * revert to normal operation.
         */
        expectedTimeOfArrival["ETA_Duration"] = srm_ptr->requests->list.array[0]->duration;
        request["expectedTimeOfArrival"] = expectedTimeOfArrival;

        /**
         * The requestor identifies itself using its current speed, heading and location.
         */
        std::stringstream ss;
        ss << srm_ptr->requestor.id.choice.entityID.buf;
        request["vehicleID"] = ss.str();
        request["vehicleType"] = srm_ptr->requestor.type->hpmsType;
        request["heading_Degree"] = srm_ptr->requestor.position->heading;
        request["speed_MeterPerSecond"] = srm_ptr->requestor.position->speed->speed;

        Json::Value position;
        position["latitude_DecimalDegree"] = srm_ptr->requestor.position->position.lat;
        position["longitude_DecimalDegree"] = srm_ptr->requestor.position->position.Long;
        position["elevation_Meter"] = srm_ptr->requestor.position->position.elevation;
        request["position"] = position;

        json["SignalRequest"] = request;
    }
}
