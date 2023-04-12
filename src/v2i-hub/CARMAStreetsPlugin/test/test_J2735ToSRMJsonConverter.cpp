#include <gtest/gtest.h>
#include <J2735ToSRMJsonConverter.h>
#include <cassert>

class test_J2735ToSRMJsonConverter : public ::testing::Test
{

public:
    test_J2735ToSRMJsonConverter() = default;
    ~test_J2735ToSRMJsonConverter() = default;

protected:
    tmx::messages::SrmMessage *_srmMessage;
    SignalRequestMessage_t *_message;
    void SetUp() override
    {
        _message = (SignalRequestMessage_t *)calloc(1, sizeof(SignalRequestMessage_t));
        _message->second = 12;
        RequestorDescription_t *requestor = (RequestorDescription_t *)calloc(1, sizeof(RequestorDescription_t));
        VehicleID_t *veh_id = (VehicleID_t *)calloc(1, sizeof(VehicleID_t));
        veh_id->present = VehicleID_PR_entityID;
        TemporaryID_t *entity_id = (TemporaryID_t *)calloc(1, sizeof(TemporaryID_t));
        uint8_t my_bytes_id[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
        entity_id->buf = my_bytes_id;
        entity_id->size = sizeof(my_bytes_id);
        veh_id->choice.entityID = *entity_id;
        requestor->id = *veh_id;
        RequestorType_t *requestType = (RequestorType_t *)calloc(1, sizeof(RequestorType_t));
        requestType->role = 0;
        requestor->type = requestType;
        RequestorPositionVector_t *position = (RequestorPositionVector_t *)calloc(1, sizeof(RequestorPositionVector_t));
        DSRC_Angle_t *heading_angle = (DSRC_Angle_t *)calloc(1, sizeof(DSRC_Angle_t));
        *heading_angle = 123;
        position->heading = heading_angle;
        Position3D_t *position_point = (Position3D_t *)calloc(1, sizeof(Position3D_t));
        DSRC_Elevation_t *elev = (DSRC_Elevation_t *)calloc(1, sizeof(DSRC_Elevation_t));
        *elev = 12;
        position_point->elevation = elev;
        position_point->lat = 3712333;
        position_point->Long = 8012333;
        position->position = *position_point;
        TransmissionAndSpeed_t *speed = (TransmissionAndSpeed_t *)calloc(1, sizeof(TransmissionAndSpeed_t));
        speed->speed = 10;
        TransmissionState_t *transmission_state = (TransmissionState_t *)calloc(1, sizeof(TransmissionState_t));
        *transmission_state = 1111;
        speed->transmisson = 7;
        position->speed = speed;
        requestor->position = position;
        _message->requestor = *requestor;

        SignalRequestList_t *requests = (SignalRequestList_t *)calloc(1, sizeof(SignalRequestList_t));
        SignalRequestPackage_t *request_package = (SignalRequestPackage_t *)calloc(1, sizeof(SignalRequestPackage_t));
        MinuteOfTheYear_t *min = (MinuteOfTheYear_t *)calloc(1, sizeof(MinuteOfTheYear_t));
        *min = 123;
        request_package->minute = min;
        DSecond_t *duration = (DSecond_t *)calloc(1, sizeof(DSecond_t));
        *duration = 122;
        request_package->duration = duration;
        DSecond_t *second = (DSecond_t *)calloc(1, sizeof(DSecond_t));
        *second = 1212;
        request_package->second = second;
        SignalRequest_t *request = (SignalRequest_t *)calloc(1, sizeof(SignalRequest_t));
        IntersectionReferenceID_t *refer_id = (IntersectionReferenceID_t *)calloc(1, sizeof(IntersectionReferenceID_t));
        refer_id->id = 1222;
        request->id = *refer_id;
        request->requestID = 1;
        request->requestType = 0;
        IntersectionAccessPoint_t *inBoundLane = (IntersectionAccessPoint_t *)calloc(1, sizeof(IntersectionAccessPoint_t));
        inBoundLane->present = IntersectionAccessPoint_PR_lane;
        inBoundLane->choice.lane = 1;
        request->inBoundLane = *inBoundLane;
        request_package->request = *request;
        asn_sequence_add(&requests->list.array, request_package);
        _message->requests = requests;
        tmx::messages::SrmEncodedMessage srmEncodeMessage;
        _srmMessage = new tmx::messages::SrmMessage(_message);
    }
};

namespace unit_test
{
    TEST_F(test_J2735ToSRMJsonConverter, toSRMJson)
    {
        CARMAStreetsPlugin::J2735ToSRMJsonConverter srmConverter;
        std::vector<Json::Value> srmJsonV;
        srmConverter.toSRMJsonV(srmJsonV, _srmMessage);
        int expectedSrmSize = 1;
        ASSERT_EQ(expectedSrmSize, srmJsonV.size());
        for (auto srmJson : srmJsonV)
        {
            Json::FastWriter fastWriter;
            std::string message = fastWriter.write(srmJson);
            std::string expectedSrmStr =  "{\"MsgType\":\"SRM\",\"SignalRequest\":{\"basicVehicleRole\":0,\"expectedTimeOfArrival\":{\"ETA_Duration\":122,\"ETA_Minute\":123,\"ETA_Second\":1212},\"heading_Degree\":123,\"inBoundLane\":{\"LaneID\":1},\"intersectionID\":1222,\"minuteOfYear\":123,\"msOfMinute\":1212,\"msgCount\":1,\"position\":{\"elevation_Meter\":12,\"latitude_DecimalDegree\":3712333,\"longitude_DecimalDegree\":8012333},\"priorityRequestType\":1,\"regionalID\":0,\"speed_MeterPerSecond\":10,\"vehicleID\":\"10c0c0a\"}}\n";
            ASSERT_EQ(expectedSrmStr, message);
        }
    }

    TEST_F(test_J2735ToSRMJsonConverter, toSRMJsonNULLObj)
    {
        CARMAStreetsPlugin::J2735ToSRMJsonConverter srmConverter;
        std::vector<Json::Value> invalidSRMJson;
        SignalRequestMessage_t *message = (SignalRequestMessage_t *)calloc(1, sizeof(SignalRequestMessage_t));
        auto invalidSRMmMessage = new tmx::messages::SrmMessage(message);
        srmConverter.toSRMJsonV(invalidSRMJson, invalidSRMmMessage);
        int expectedSrmSize = 0;
        ASSERT_EQ(expectedSrmSize, invalidSRMJson.size());
    }
}
