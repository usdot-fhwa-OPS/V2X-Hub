#include <gtest/gtest.h>
#include <J2735ToSRMJsonConverter.h>
#include <cassert>
#if SAEJ2735_SPEC < 2020
using MsgCount_t = DSRC_MsgCount_t;
#else
using MsgCount_t = Common_MsgCount_t;
#endif
class test_J2735ToSRMJsonConverter : public ::testing::Test
{

public:
    test_J2735ToSRMJsonConverter() = default;
    ~test_J2735ToSRMJsonConverter() = default;

protected:
    tmx::messages::SrmMessage _srmMessage;
    std::shared_ptr<SignalRequestMessage_t> _message;
    void SetUp() override
    {
        _message = tmx::messages::j2735::j2735_create<tmx::messages::SrmTraits>();
        _message->second = 1;
        _message->sequenceNumber = tmx::messages::j2735::AllocAsn<MsgCount_t>();
        *_message->sequenceNumber = 123;
        RequestorDescription_t requestor;
        memset(&requestor, 0, sizeof(requestor));

        // Initialize zeroed VehicleID_t struct to avoid uninitialized memory issues
        VehicleID_t veh_id;
        memset(&veh_id, 0, sizeof(veh_id));
        veh_id.present = VehicleID_PR_entityID;
        char my_bytes_id[4] = {(char)24, (char)12, (char)12, (char)10};
        // TempId is octet string and can be populated from buffer
        bool failed = OCTET_STRING_fromBuf(&veh_id.choice.entityID, my_bytes_id, sizeof(my_bytes_id));
        // If operation fails, unit test is no longer valid
        ASSERT_EQ(failed, 0);
        requestor.id = veh_id;
        RequestorType_t *requestType = tmx::messages::j2735::AllocAsn<RequestorType_t>();
        requestType->role = 0;
        requestor.type = requestType;
        RequestorPositionVector_t *position = tmx::messages::j2735::AllocAsn<RequestorPositionVector_t>();
        #if SAEJ2735_SPEC < 2020
        DSRC_Angle_t *heading_angle = tmx::messages::j2735::AllocAsn<DSRC_Angle_t>();
        #else
        Common_Angle_t *heading_angle = tmx::messages::j2735::AllocAsn<Common_Angle_t>();
        #endif
        *heading_angle = 123;
        position->heading = heading_angle;
        Position3D_t position_point;
        memset(&position_point, 0, sizeof(position_point));
        #if SAEJ2735_SPEC < 2020
        DSRC_Elevation_t *elev = tmx::messages::j2735::AllocAsn<DSRC_Elevation_t>();
        #else
        Common_Elevation_t *elev = tmx::messages::j2735::AllocAsn<Common_Elevation_t>();
        #endif
        *elev = 12;
        position_point.elevation = elev;
        position_point.lat = 3712333;
        position_point.Long = 8012333;
        position->position = position_point;
        TransmissionAndSpeed_t *speed = tmx::messages::j2735::AllocAsn<TransmissionAndSpeed_t>();
        speed->speed = 10;
        speed->transmisson = 7;
        position->speed = speed;
        requestor.position = position;
        _message->requestor = requestor;

        SignalRequestList_t *requests = tmx::messages::j2735::AllocAsn<SignalRequestList_t>();
        //First: Request Package
        SignalRequestPackage_t *request_package = tmx::messages::j2735::AllocAsn<SignalRequestPackage_t>();
        {
            MinuteOfTheYear_t *min = tmx::messages::j2735::AllocAsn<MinuteOfTheYear_t>();
            *min = 123;
            request_package->minute = min;
            DSecond_t *duration = tmx::messages::j2735::AllocAsn<DSecond_t>();
            *duration = 122;
            request_package->duration = duration;
            DSecond_t *second = tmx::messages::j2735::AllocAsn<DSecond_t>();
            *second = 1212;
            request_package->second = second;
        }
        request_package->request.id.id = 1222;
        request_package->request.requestID = 1;
        request_package->request.requestType = 0;

        request_package->request.inBoundLane.present = IntersectionAccessPoint_PR_lane;
        request_package->request.inBoundLane.choice.lane = 1;
        asn_sequence_add(&requests->list.array, request_package);

        //Second: Request Package
        SignalRequestPackage_t *request_package_2 = tmx::messages::j2735::AllocAsn<SignalRequestPackage_t>();
        {
            MinuteOfTheYear_t *min = tmx::messages::j2735::AllocAsn<MinuteOfTheYear_t>();
            *min = 123;
            DSecond_t *duration = tmx::messages::j2735::AllocAsn<DSecond_t>();
            *duration = 122;
            DSecond_t *second = tmx::messages::j2735::AllocAsn<DSecond_t>();
            *second = 1212;
            request_package_2->minute = min;
            request_package_2->duration = duration;
            request_package_2->second = second;
        }
        
        request_package_2->request.id.id = 2333;
        request_package_2->request.requestID = 2;
        request_package_2->request.requestType = 1;
        request_package_2->request.inBoundLane.present = IntersectionAccessPoint_PR_approach;
        request_package_2->request.inBoundLane.choice.approach = 1;
        asn_sequence_add(&requests->list.array, request_package_2);
        _message->requests = requests;
        tmx::messages::SrmEncodedMessage srmEncodeMessage;
        _srmMessage = tmx::messages::SrmMessage(_message);

    }
};

namespace unit_test
{
    TEST_F(test_J2735ToSRMJsonConverter, toSRMJson)
    {
        CARMAStreetsPlugin::J2735ToSRMJsonConverter srmConverter;
        std::vector<Json::Value> srmJsonV;
        srmConverter.toSRMJsonV(srmJsonV, &_srmMessage);
        int expectedSrmSize = 2;
        ASSERT_EQ(expectedSrmSize, srmJsonV.size());
        int i = 0;
        for (const auto &srmJson : srmJsonV)
        {
            Json::FastWriter fastWriter;
            std::string message = fastWriter.write(srmJson);
            std::string expectedSrmStr = "";
            if (i == 0)
            {
                expectedSrmStr = "{\"MsgType\":\"SRM\",\"SignalRequest\":{\"basicVehicleRole\":0,\"expectedTimeOfArrival\":{\"ETA_Duration\":122,\"ETA_Minute\":123,\"ETA_Second\":1212},\"heading_Degree\":123,\"inBoundLane\":{\"LaneID\":1},\"intersectionID\":1222,\"minuteOfYear\":123,\"msOfMinute\":1212,\"msgCount\":123,\"position\":{\"elevation_Meter\":120,\"latitude_DecimalDegree\":0.37123331427574158,\"longitude_DecimalDegree\":0.80123329162597656},\"priorityRequestType\":0,\"speed_MeterPerSecond\":500.0,\"vehicleID\":168561688}}\n";
            }
            else if (i == 1)
            {
                expectedSrmStr = "{\"MsgType\":\"SRM\",\"SignalRequest\":{\"basicVehicleRole\":0,\"expectedTimeOfArrival\":{\"ETA_Duration\":122,\"ETA_Minute\":123,\"ETA_Second\":1212},\"heading_Degree\":123,\"inBoundLane\":{\"ApproachID\":1},\"intersectionID\":2333,\"minuteOfYear\":123,\"msOfMinute\":1212,\"msgCount\":123,\"position\":{\"elevation_Meter\":120,\"latitude_DecimalDegree\":0.37123331427574158,\"longitude_DecimalDegree\":0.80123329162597656},\"priorityRequestType\":1,\"speed_MeterPerSecond\":500.0,\"vehicleID\":168561688}}\n";
            }
            ASSERT_EQ(expectedSrmStr, message);
            i++;
        }
    }

    TEST_F(test_J2735ToSRMJsonConverter, toSRMJsonNULLObj)
    {
        CARMAStreetsPlugin::J2735ToSRMJsonConverter srmConverter;
        std::vector<Json::Value> invalidSRMJson;
        std::shared_ptr<SignalRequestMessage_t> message = tmx::messages::j2735::j2735_create<tmx::messages::SrmTraits>();
        auto invalidSRMmMessage = tmx::messages::SrmMessage(message);
        srmConverter.toSRMJsonV(invalidSRMJson, &invalidSRMmMessage);
        int expectedSrmSize = 0;
        EXPECT_EQ(expectedSrmSize, invalidSRMJson.size());
    }
}
