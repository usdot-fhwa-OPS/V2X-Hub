#include "J2735ToSRMJsonConverter.h"

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

    Json::Value request;
    request["msgCount"] = 1;
    request["minuteOfYear"] = 345239;
    request["msOfMinute"] = 54000;
    request["regionalID"] = 0;
    request["intersectionID"] = srm_ptr->requests->list.array[0]->request.id.id;
    request["priorityRequestType"] =   srm_ptr->requests->list.array[0]->request.requestID;
    std::stringstream ss;
    ss << srm_ptr->requestor.id.choice.entityID.buf;
    request["vehicleID"] = ss.str();
    request["basicVehicleRole"] =  srm_ptr->requestor.type->role;
    request["vehicleType"] = srm_ptr->requestor.type->hpmsType;
    request["heading_Degree"] = srm_ptr->requestor.position->heading;
    request["speed_MeterPerSecond"] = srm_ptr->requestor.position->speed->speed;

    Json::Value inBoundLane;
    inBoundLane["LaneID"] = srm_ptr->requests->list.array[0]->request.inBoundLane.choice.lane;
    // inBoundLane["ApproachID"] = srm_ptr->requests->list.array[0]->request.inBoundLane.choice.approach;
    request["inBoundLane"] = inBoundLane;

    Json::Value expectedTimeOfArrival;
    expectedTimeOfArrival["ETA_Minute"] = srm_ptr->requests->list.array[0]->minute;
    expectedTimeOfArrival["ETA_Second"] = srm_ptr->requests->list.array[0]->second;
    expectedTimeOfArrival["ETA_Duration"] = srm_ptr->requests->list.array[0]->duration;
    request["expectedTimeOfArrival"] = expectedTimeOfArrival;

    Json::Value position;
    position["latitude_DecimalDegree"] =  srm_ptr->requestor.position->position.lat;
    position["longitude_DecimalDegree"] = srm_ptr->requestor.position->position.Long;
    position["elevation_Meter"] = srm_ptr->requestor.position->position.elevation;
    request["position"] = position;

    json["SignalRequest"] = request;
    std::cout << json.toStyledString() << std::endl;
}
