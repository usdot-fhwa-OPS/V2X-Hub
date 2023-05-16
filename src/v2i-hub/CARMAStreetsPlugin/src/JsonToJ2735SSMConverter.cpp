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
            IntersectionReferenceID_t *id = (IntersectionReferenceID_t *)calloc(1, sizeof(IntersectionReferenceID_t));
            id->id = ssmDoc["SignalStatus"]["intersectionID"].asInt64();
            signalStatus->id = *id;
        }

        // populate SignalStatusMessage::status::sequenceNumber
        if (ssmDoc["SignalStatus"].isMember("sequenceNumber") && ssmDoc["SignalStatus"]["sequenceNumber"].isNumeric())
        {
            signalStatus->sequenceNumber = ssmDoc["SignalStatus"]["sequenceNumber"].asInt64();
        }

        SignalStatusPackageList_t *sigStatus = (SignalStatusPackageList_t *)calloc(1, sizeof(SignalStatusPackageList_t));
        // populate SignalStatusMessage::status::sigStatus
        if (ssmDoc["SignalStatus"].isMember("requestorInfo") && ssmDoc["SignalStatus"]["requestorInfo"].isArray())
        {
            Json::Value requesterJsonArr = ssmDoc["SignalStatus"]["requestorInfo"];
            for (auto itr = requesterJsonArr.begin(); itr != requesterJsonArr.end(); itr++)
            {
                SignalStatusPackage *signalStatusPackage = (SignalStatusPackage *)calloc(1, sizeof(SignalStatusPackage));
                populateSigStatusPackage(signalStatusPackage, itr);
                asn_sequence_add(&sigStatus->list.array, signalStatusPackage);
            } // Populate signal status package
        }

        signalStatus->sigStatus = *sigStatus;
        asn_sequence_add(&statusPtr->list.array, signalStatus);
        ssmPtr->status = *statusPtr;
    }
 
    void JsonToJ2735SSMConverter::populateSigStatusPackage(SignalStatusPackage *signalStatusPackage, Json::Value::iterator itr) const
    {
        SignalRequesterInfo *requester = (SignalRequesterInfo *)calloc(1, sizeof(SignalRequesterInfo));

        // populate SignalStatusMessage::status::sigStatus::requester::request
        if (itr->isMember("requestID") && (*itr)["requestID"].isNumeric())
        {
            requester->request = (*itr)["requestID"].asInt64();
        }

        // populate SignalStatusMessage::status::sigStatus::requester::id
        if (itr->isMember("vehicleID") && (*itr)["vehicleID"].isNumeric())
        {
            VehicleID *vehID = (VehicleID *)calloc(1, sizeof(VehicleID));
            vehID->choice.stationID = (*itr)["vehicleID"].asInt64();
            vehID->present = VehicleID_PR_stationID;
            requester->id = *vehID;
        }

        // populate SignalStatusMessage::status::sigStatus::requester::sequenceNumber
        if (itr->isMember("msgCount") && (*itr)["msgCount"].isNumeric())
        {
            requester->sequenceNumber = (*itr)["msgCount"].asInt64();
        }

        // populate SignalStatusMessage::status::sigStatus::requester::role
        if (itr->isMember("basicVehicleRole") && (*itr)["basicVehicleRole"].isNumeric())
        {
            BasicVehicleRole_t *role = (BasicVehicleRole_t *)calloc(1, sizeof(BasicVehicleRole_t));
            *role = (*itr)["basicVehicleRole"].asInt64();
            requester->role = role;
        }

        signalStatusPackage->requester = requester;

        // populate SignalStatusMessage::status::sigStatus::inboundOn
        if (itr->isMember("inBoundLaneID") && (*itr)["inBoundLaneID"].isNumeric())
        {
            IntersectionAccessPoint_t *inboundOn = (IntersectionAccessPoint_t *)calloc(1, sizeof(IntersectionAccessPoint_t));
            inboundOn->present = IntersectionAccessPoint_PR_lane;
            inboundOn->choice.lane = (*itr)["inBoundLaneID"].asInt64();
            signalStatusPackage->inboundOn = *inboundOn;
        }
        else if (itr->isMember("inBoundApproachID") && (*itr)["inBoundApproachID"].isNumeric())
        {
            IntersectionAccessPoint_t *inboundOn = (IntersectionAccessPoint_t *)calloc(1, sizeof(IntersectionAccessPoint_t));
            inboundOn->present = IntersectionAccessPoint_PR_approach;
            inboundOn->choice.approach = (*itr)["inBoundApproachID"].asInt64();
            signalStatusPackage->inboundOn = *inboundOn;
        }

        // populate SignalStatusMessage::status::sigStatus::status
        if (itr->isMember("priorityRequestStatus") && (*itr)["priorityRequestStatus"].isNumeric())
        {
            PrioritizationResponseStatus_t *status = (PrioritizationResponseStatus_t *)calloc(1, sizeof(PrioritizationResponseStatus_t));
            *status = (*itr)["priorityRequestStatus"].asInt64();
            signalStatusPackage->status = *status;
        }

        // populate SignalStatusMessage::status::sigStatus::duration
        if (itr->isMember("ETA_Duration") && (*itr)["ETA_Duration"].isNumeric())
        {
            DSecond_t *duration = (DSecond_t *)calloc(1, sizeof(DSecond_t));
            *duration = (*itr)["ETA_Duration"].asInt64();
            signalStatusPackage->duration = duration;
        }

        // populate SignalStatusMessage::status::sigStatus::minute
        if (itr->isMember("ETA_Minute") && (*itr)["ETA_Minute"].isNumeric())
        {
            DSecond_t *minute = (DSecond_t *)calloc(1, sizeof(DSecond_t));
            *minute = (*itr)["ETA_Minute"].asInt64();
            signalStatusPackage->minute = minute;
        }

        // populate SignalStatusMessage::status::sigStatus::second
        if (itr->isMember("ETA_Second") && (*itr)["ETA_Second"].isNumeric())
        {
            DSecond_t *second = (DSecond_t *)calloc(1, sizeof(DSecond_t));
            *second = (*itr)["ETA_Second"].asInt64();
            signalStatusPackage->second = second;
        }
    }
    void JsonToJ2735SSMConverter::encodeSSM(const std::shared_ptr<SignalStatusMessage> &ssmPtr, tmx::messages::SsmEncodedMessage &encodedSSM) const
    {
        tmx::messages::MessageFrameMessage frame(ssmPtr);
        encodedSSM.set_data(tmx::messages::TmxJ2735EncodedMessage<SignalStatusMessage>::encode_j2735_message<tmx::messages::codec::uper<tmx::messages::MessageFrameMessage>>(frame));
        free(frame.get_j2735_data().get());
    }
}
