#include "JsonToJ2735SSMConverter.h"

namespace CARMAStreetsPlugin
{

    bool JsonToJ2735SSMConverter::parseJsonString(const string &consumedMsg, Json::Value &ssmDoc) const
    {
        const auto jsonLen = static_cast<int>(consumedMsg.length());
        Json::CharReaderBuilder builder;
        JSONCPP_STRING err;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        bool parseResult = reader->parse(consumedMsg.c_str(), consumedMsg.c_str() + jsonLen, &ssmDoc, &err);
        if (!parseResult)
        {
            PLOG(logERROR) << "Parse error: " << err << endl;
        }
        return parseResult;
    }

    void JsonToJ2735SSMConverter::toJ2735SSM(const Json::Value &ssmDoc, std::shared_ptr<SignalStatusMessage> ssmPtr) const
    {
        try 
        {        
            if (!ssmDoc.isMember("SignalStatus"))
            {
                PLOG(logERROR) << "No SignalStatus present in JSON."  << std::endl;
                return;
            }

            // populate SignalStatusMessage::second
            if (ssmDoc["SignalStatus"].isMember("msOfMinute") && ssmDoc["SignalStatus"]["msOfMinute"].isNumeric())
            {
                ssmPtr->second = ssmDoc["SignalStatus"]["msOfMinute"].asInt64();
            }

            // populate SignalStatusMessage::timstamp
            if (ssmDoc["SignalStatus"].isMember("minuteOfYear") && ssmDoc["SignalStatus"]["minuteOfYear"].isNumeric())
            {
                MinuteOfTheYear_t *timeStamp = tmx::messages::j2735::AllocAsn<MinuteOfTheYear_t>();
                *timeStamp = ssmDoc["SignalStatus"]["minuteOfYear"].asInt64();
                ssmPtr->timeStamp = timeStamp;
            }

            SignalStatus *signalStatus = tmx::messages::j2735::AllocAsn<SignalStatus>();

            // populate SignalStatusMessage::status::id
            if (ssmDoc["SignalStatus"].isMember("intersectionID") && ssmDoc["SignalStatus"]["intersectionID"].isNumeric())
            {
                signalStatus->id.id = ssmDoc["SignalStatus"]["intersectionID"].asInt64();
            }

            // populate SignalStatusMessage::status::sequenceNumber
            if (ssmDoc["SignalStatus"].isMember("sequenceNumber") && ssmDoc["SignalStatus"]["sequenceNumber"].isNumeric())
            {
                signalStatus->sequenceNumber = ssmDoc["SignalStatus"]["sequenceNumber"].asInt64();
            }

            // populate SignalStatusMessage::status::sigStatus
            if (ssmDoc["SignalStatus"].isMember("requestorInfo") && ssmDoc["SignalStatus"]["requestorInfo"].isArray())
            {
                Json::Value requesterJsonArr = ssmDoc["SignalStatus"]["requestorInfo"];
                for (auto itr = requesterJsonArr.begin(); itr != requesterJsonArr.end(); itr++)
                {
                    SignalStatusPackage *signalStatusPackage = tmx::messages::j2735::AllocAsn<SignalStatusPackage>();
                    populateSigStatusPackage(signalStatusPackage, itr);
                    asn_sequence_add(&signalStatus->sigStatus.list.array, signalStatusPackage);
                } // Populate signal status package
            }

            asn_sequence_add(&ssmPtr->status.list.array, signalStatus);
        }
        catch(const exception &ex)
        {
            PLOG(logERROR) << "Cannot read JSON file."  << std::endl;
        }
    }
 
    void JsonToJ2735SSMConverter::populateSigStatusPackage(SignalStatusPackage *signalStatusPackage, Json::Value::iterator itr) const
    {
        signalStatusPackage->requester  = tmx::messages::j2735::AllocAsn<SignalRequesterInfo>();

        // populate SignalStatusMessage::status::sigStatus::requester::request
        if (itr->isMember("requestID") && (*itr)["requestID"].isNumeric())
        {
            signalStatusPackage->requester->request = (*itr)["requestID"].asInt64();
        }

        // populate SignalStatusMessage::status::sigStatus::requester::id
        if (itr->isMember("vehicleID") && (*itr)["vehicleID"].isNumeric())
        {
            signalStatusPackage->requester->id.choice.stationID = (*itr)["vehicleID"].asInt64();
            signalStatusPackage->requester->id.present = VehicleID_PR_stationID;
        }

        // populate SignalStatusMessage::status::sigStatus::requester::sequenceNumber
        if (itr->isMember("msgCount") && (*itr)["msgCount"].isNumeric())
        {
            signalStatusPackage->requester->sequenceNumber = (*itr)["msgCount"].asInt64();
        }

        // populate SignalStatusMessage::status::sigStatus::requester::role
        if (itr->isMember("basicVehicleRole") && (*itr)["basicVehicleRole"].isNumeric())
        {
            signalStatusPackage->requester->role = tmx::messages::j2735::AllocAsn<BasicVehicleRole_t>();
            *signalStatusPackage->requester->role  = (*itr)["basicVehicleRole"].asInt64();
        }

        // populate SignalStatusMessage::status::sigStatus::inboundOn
        if (itr->isMember("inBoundLaneID") && (*itr)["inBoundLaneID"].isNumeric())
        {
            signalStatusPackage->inboundOn.present = IntersectionAccessPoint_PR_lane;
            signalStatusPackage->inboundOn.choice.lane = (*itr)["inBoundLaneID"].asInt64();
        }
        else if (itr->isMember("inBoundApproachID") && (*itr)["inBoundApproachID"].isNumeric())
        {
            signalStatusPackage->inboundOn.present = IntersectionAccessPoint_PR_approach;
            signalStatusPackage->inboundOn.choice.approach = (*itr)["inBoundApproachID"].asInt64();
        }

        // populate SignalStatusMessage::status::sigStatus::status
        if (itr->isMember("priorityRequestStatus") && (*itr)["priorityRequestStatus"].isNumeric())
        {
            signalStatusPackage->status = (*itr)["priorityRequestStatus"].asInt64();
        }

        // populate SignalStatusMessage::status::sigStatus::duration
        if (itr->isMember("ETA_Duration") && (*itr)["ETA_Duration"].isNumeric())
        {
            signalStatusPackage->duration = tmx::messages::j2735::AllocAsn<DSecond_t>();
            *signalStatusPackage->duration = (*itr)["ETA_Duration"].asInt64();
        }

        // populate SignalStatusMessage::status::sigStatus::minute
        if (itr->isMember("ETA_Minute") && (*itr)["ETA_Minute"].isNumeric())
        {
            signalStatusPackage->minute = tmx::messages::j2735::AllocAsn<DSecond_t>();
            *signalStatusPackage->minute = (*itr)["ETA_Minute"].asInt64();
        }

        // populate SignalStatusMessage::status::sigStatus::second
        if (itr->isMember("ETA_Second") && (*itr)["ETA_Second"].isNumeric())
        {
            signalStatusPackage->second = tmx::messages::j2735::AllocAsn<DSecond_t>();
            *signalStatusPackage->second = (*itr)["ETA_Second"].asInt64();
        }
    }
    void JsonToJ2735SSMConverter::encodeSSM(const std::shared_ptr<SignalStatusMessage> &ssmPtr, tmx::messages::SsmEncodedMessage &encodedSSM) const
    {
        tmx::messages::MessageFrameMessage frame(ssmPtr);
        encodedSSM.set_data(tmx::messages::TmxJ2735EncodedMessage<SignalStatusMessage>::encode_j2735_message<tmx::messages::codec::uper<tmx::messages::MessageFrameMessage>>(frame));
    }
}
