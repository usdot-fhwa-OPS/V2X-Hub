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
            std::cout << "Parse error: " << err << endl;
        }
        return parseResult;
    }

    void JsonToJ2735SSMConverter::toJ2735SSM(const Json::Value &ssmDoc, std::shared_ptr<SignalStatusMessage> ssmPtr) const
    {
        try 
        {        
            ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());
            if (!ssmDoc.isMember("SignalStatus"))
            {
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
                MinuteOfTheYear_t *timeStamp = (MinuteOfTheYear_t *)calloc(1, sizeof(MinuteOfTheYear_t));
                *timeStamp = ssmDoc["SignalStatus"]["minuteOfYear"].asInt64();
                ssmPtr->timeStamp = timeStamp;
            }

            SignalStatusList_t *statusPtr = (SignalStatusList_t *)calloc(1, sizeof(SignalStatusList_t));
            SignalStatus *signalStatus = (SignalStatus *)calloc(1, sizeof(SignalStatus));

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
                    SignalStatusPackage *signalStatusPackage = (SignalStatusPackage *)calloc(1, sizeof(SignalStatusPackage));
                    populateSigStatusPackage(signalStatusPackage, itr);
                    asn_sequence_add(&signalStatus->sigStatus.list.array, signalStatusPackage);
                } // Populate signal status package
            }

            asn_sequence_add(&statusPtr->list.array, signalStatus);
            ssmPtr->status = *statusPtr;
        }
        catch(exception &ex)
        {
            std::cout << "ERROR: Cannot read JSON file." << std::endl;
        }
    }
 
    void JsonToJ2735SSMConverter::populateSigStatusPackage(SignalStatusPackage *signalStatusPackage, Json::Value::iterator itr) const
    {
        signalStatusPackage->requester  = (SignalRequesterInfo *)calloc(1, sizeof(SignalRequesterInfo));

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
            signalStatusPackage->requester->role = (BasicVehicleRole_t *)calloc(1, sizeof(BasicVehicleRole_t));
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
            signalStatusPackage->duration = (DSecond_t *)calloc(1, sizeof(DSecond_t));
            *signalStatusPackage->duration = (*itr)["ETA_Duration"].asInt64();
            // signalStatusPackage->duration = duration;
        }

        // populate SignalStatusMessage::status::sigStatus::minute
        if (itr->isMember("ETA_Minute") && (*itr)["ETA_Minute"].isNumeric())
        {
            signalStatusPackage->minute = (DSecond_t *)calloc(1, sizeof(DSecond_t));
            *signalStatusPackage->minute = (*itr)["ETA_Minute"].asInt64();
        }

        // populate SignalStatusMessage::status::sigStatus::second
        if (itr->isMember("ETA_Second") && (*itr)["ETA_Second"].isNumeric())
        {
            signalStatusPackage->minute = (DSecond_t *)calloc(1, sizeof(DSecond_t));
            *signalStatusPackage->minute = (*itr)["ETA_Second"].asInt64();
        }
    }
    void JsonToJ2735SSMConverter::encodeSSM(const std::shared_ptr<SignalStatusMessage> &ssmPtr, tmx::messages::SsmEncodedMessage &encodedSSM) const
    {
        tmx::messages::MessageFrameMessage frame(ssmPtr);
        encodedSSM.set_data(tmx::messages::TmxJ2735EncodedMessage<SignalStatusMessage>::encode_j2735_message<tmx::messages::codec::uper<tmx::messages::MessageFrameMessage>>(frame));
        free(frame.get_j2735_data().get());
    }
}
