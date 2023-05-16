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

        if (ssmDoc["SignalStatus"].isMember("msOfMinute"))
        {
            ssmPtr->second = ssmDoc["SignalStatus"]["msOfMinute"].asInt64();
        }

        if (ssmDoc["SignalStatus"].isMember("minuteOfYear"))
        {
            MinuteOfTheYear_t *timeStamp = (MinuteOfTheYear_t *)calloc(1, sizeof(MinuteOfTheYear_t));
            *timeStamp = ssmDoc["SignalStatus"]["minuteOfYear"].asInt64();
            ssmPtr->timeStamp = timeStamp;
        }

        SignalStatusList_t *statusPtr = (SignalStatusList_t *)calloc(1, sizeof(SignalStatusList_t));
        SignalStatus *signalStatus = (SignalStatus *)calloc(1, sizeof(SignalStatus));
        IntersectionReferenceID_t *id = (IntersectionReferenceID_t *)calloc(1, sizeof(IntersectionReferenceID_t));
        id->id = ssmDoc["SignalStatus"]["intersectionID"].asInt64();
        signalStatus->id = *id;
        signalStatus->sequenceNumber = ssmDoc["SignalStatus"]["sequenceNumber"].asInt64();
        SignalStatusPackageList_t *sigStatus = (SignalStatusPackageList_t *)calloc(1, sizeof(SignalStatusPackageList_t));

        if (ssmDoc["SignalStatus"].isMember("requestorInfo") && ssmDoc["SignalStatus"]["requestorInfo"].isArray())
        {
            Json::Value requesterJsonArr = ssmDoc["SignalStatus"]["requestorInfo"];
            for (auto itr = requesterJsonArr.begin(); itr != requesterJsonArr.end(); itr++)
            {
                SignalStatusPackage *signalStatusPackage = (SignalStatusPackage *)calloc(1, sizeof(SignalStatusPackage));
                SignalRequesterInfo *requester = (SignalRequesterInfo *)calloc(1, sizeof(SignalRequesterInfo));
                if (itr->isMember("requestID"))
                {
                    requester->request = (*itr)["requestID"].asInt64();
                }

                if (itr->isMember("vehicleID"))
                {
                    VehicleID *vehID = (VehicleID *)calloc(1, sizeof(VehicleID));
                    vehID->choice.stationID = (*itr)["vehicleID"].asInt64();
                    vehID->present = VehicleID_PR_stationID;
                    requester->id = *vehID;
                    // ToDo for string
                }

                if (itr->isMember("msgCount"))
                {
                    requester->sequenceNumber = (*itr)["msgCount"].asInt64();
                }

                if (itr->isMember("basicVehicleRole"))
                {
                    BasicVehicleRole_t *role = (BasicVehicleRole_t *)calloc(1, sizeof(BasicVehicleRole_t));
                    *role = (*itr)["basicVehicleRole"].asInt64();
                    requester->role = role;
                }

                signalStatusPackage->requester = requester;

                if (itr->isMember("inBoundLaneID"))
                {
                    IntersectionAccessPoint_t *inboundOn = (IntersectionAccessPoint_t *)calloc(1, sizeof(IntersectionAccessPoint_t));
                    inboundOn->present = IntersectionAccessPoint_PR_lane;
                    inboundOn->choice.lane = (*itr)["inBoundLaneID"].asInt64();
                    signalStatusPackage->inboundOn = *inboundOn;
                }

                if (itr->isMember("priorityRequestStatus"))
                {
                    PrioritizationResponseStatus_t *status = (PrioritizationResponseStatus_t *)calloc(1, sizeof(PrioritizationResponseStatus_t));
                    *status = (*itr)["priorityRequestStatus"].asInt64();
                    signalStatusPackage->status = *status;
                }

                if (itr->isMember("ETA_Duration"))
                {
                    DSecond_t *duration = (DSecond_t *)calloc(1, sizeof(DSecond_t));
                    *duration = (*itr)["ETA_Duration"].asInt64();
                    signalStatusPackage->duration = duration;
                }

                if (itr->isMember("ETA_Minute"))
                {
                    DSecond_t *minute = (DSecond_t *)calloc(1, sizeof(DSecond_t));
                    *minute = (*itr)["ETA_Minute"].asInt64();
                    signalStatusPackage->minute = minute;
                }

                if (itr->isMember("ETA_Second"))
                {
                    DSecond_t *second = (DSecond_t *)calloc(1, sizeof(DSecond_t));
                    *second = (*itr)["ETA_Second"].asInt64();
                    signalStatusPackage->second = second;
                }
                asn_sequence_add(&sigStatus->list.array, signalStatusPackage);
            } // Populate signal status package
        }

        signalStatus->sigStatus = *sigStatus;
        asn_sequence_add(&statusPtr->list.array, signalStatus);
        ssmPtr->status = *statusPtr;
    }

    void JsonToJ2735SSMConverter::encodeSSM(const std::shared_ptr<SignalStatusMessage> &ssmPtr, tmx::messages::SsmEncodedMessage &encodedSSM) const
    {
        tmx::messages::MessageFrameMessage frame(ssmPtr);
        encodedSSM.set_data(tmx::messages::TmxJ2735EncodedMessage<SignalStatusMessage>::encode_j2735_message<tmx::messages::codec::uper<tmx::messages::MessageFrameMessage>>(frame));
        free(frame.get_j2735_data().get());
    }
}
