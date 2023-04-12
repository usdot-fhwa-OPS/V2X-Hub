#include "J2735ToSRMJsonConverter.h"

namespace CARMAStreetsPlugin
{
    J2735ToSRMJsonConverter::J2735ToSRMJsonConverter()
    {
    }

    J2735ToSRMJsonConverter::~J2735ToSRMJsonConverter()
    {
    }

    void J2735ToSRMJsonConverter::toSRMJsonV(std::vector<Json::Value> &jsonV, tmx::messages::SrmMessage *srm)
    {
        if(!srm)
        {            
            return;
        }
        auto srm_ptr = srm->get_j2735_data();
        if(!srm_ptr || !srm_ptr->requests  || srm_ptr->requests->list.count <= 0)
        {
            return;
        }
        
        for(auto i = 0; i < srm_ptr->requests->list.count; i++)
        {
            Json::Value srmJson;
            srmJson["MsgType"] = MsgType;
            /***
             * Request data for one or more signalized intersections that support SRM dialogs.
             */
            Json::Value request;
            request["msgCount"] = srm_ptr->requests->list.count;
            if(srm_ptr->requests->list.array[i]->minute)
            {
                request["minuteOfYear"] = *srm_ptr->requests->list.array[i]->minute;
            }
            if(srm_ptr->requests->list.array[i]->second)
            {
                request["msOfMinute"] = *srm_ptr->requests->list.array[i]->second;
            }            
            request["regionalID"] = 0;
            request["intersectionID"] = srm_ptr->requests->list.array[i]->request.id.id;
            request["priorityRequestType"] = srm_ptr->requests->list.array[i]->request.requestID;
            request["basicVehicleRole"] = srm_ptr->requestor.type->role;

            Json::Value inBoundLane;
            switch(srm_ptr->requests->list.array[i]->request.inBoundLane.present)
            {
                case IntersectionAccessPoint_PR_NOTHING:
                    break;
                case IntersectionAccessPoint_PR_lane:
                    inBoundLane["LaneID"] = srm_ptr->requests->list.array[i]->request.inBoundLane.choice.lane;
                    break;
                case IntersectionAccessPoint_PR_approach:
                    inBoundLane["ApproachID"] = srm_ptr->requests->list.array[i]->request.inBoundLane.choice.approach;
                    break;
                case IntersectionAccessPoint_PR_connection:
                    inBoundLane["ConnectionID"] = srm_ptr->requests->list.array[i]->request.inBoundLane.choice.connection;
                    break;
            }            
            request["inBoundLane"] = inBoundLane;

            Json::Value expectedTimeOfArrival;
            // Minute of the year
            if(srm_ptr->requests->list.array[i]->minute)
            {
                expectedTimeOfArrival["ETA_Minute"] = *srm_ptr->requests->list.array[i]->minute;
            }           
            if(srm_ptr->requests->list.array[i]->second)
            {
                expectedTimeOfArrival["ETA_Second"] = *srm_ptr->requests->list.array[i]->second;
            }            
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
            if(srm_ptr->requests->list.array[i]->duration)
            {
                expectedTimeOfArrival["ETA_Duration"] = *srm_ptr->requests->list.array[i]->duration;
            }            
            request["expectedTimeOfArrival"] = expectedTimeOfArrival;

            /**
             * The requestor identifies itself using its current speed, heading and location.
             */
            std::stringstream ss;
            ss << srm_ptr->requestor.id.choice.entityID.buf;
            auto id_len = srm_ptr->requestor.id.choice.entityID.size;
            unsigned long id_num = 0;
            for(auto i = 0; i < id_len; i++)
            {			
                id_num = (id_num << 8) | srm_ptr->requestor.id.choice.entityID.buf[i];
            }
            std::stringstream id_fill_ss;
            id_fill_ss << std::hex << id_num;
            request["vehicleID"] = id_fill_ss.str();
            if(srm_ptr->requestor.type->hpmsType)
            {
                request["vehicleType"] = *srm_ptr->requestor.type->hpmsType;
            }
            if( srm_ptr->requestor.position)
            {
                if(srm_ptr->requestor.position->heading)
                {
                    request["heading_Degree"] = *srm_ptr->requestor.position->heading;
                }                
                if(srm_ptr->requestor.position->speed)
                {
                    request["speed_MeterPerSecond"] = srm_ptr->requestor.position->speed->speed;
                }                
            }           

            Json::Value position;
            position["latitude_DecimalDegree"] = srm_ptr->requestor.position->position.lat;
            position["longitude_DecimalDegree"] = srm_ptr->requestor.position->position.Long;
            if(srm_ptr->requestor.position->position.elevation)
            {
                position["elevation_Meter"] = *srm_ptr->requestor.position->position.elevation;
            }            
            request["position"] = position;
            srmJson["SignalRequest"] = request;
            jsonV.push_back(srmJson);
        }
        
    }
}
