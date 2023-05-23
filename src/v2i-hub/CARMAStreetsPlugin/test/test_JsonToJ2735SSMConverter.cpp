#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>
#include "jsoncpp/json/json.h"
#include "JsonToJ2735SSMConverter.h"
using namespace std;

namespace CARMAStreetsPlugin
{
    class test_JsonToJ2735SSMConverter : public ::testing::Test
    {
    };

    TEST_F(test_JsonToJ2735SSMConverter, parseJsonString)
    {
        JsonToJ2735SSMConverter converter;
        // Json string refer to: https://github.com/mmitss/mmitss-az/blob/master/src/mrp/priority-request-server/SSM.json
        string valid_json_str = "{\"MessageType\":\"SSM\",\"SignalStatus\":{\"intersectionID\":1003,\"minuteOfYear\":345239,\"msOfMinute\":54000,\"regionalID\":0,\"requestorInfo\":[{\"ETA_Duration\":2000,\"ETA_Minute\":1,\"ETA_Second\":20.001000000000005,\"basicVehicleRole\":16,\"inBoundLaneID\":8,\"msgCount\":2,\"priorityRequestStatus\":4,\"requestID\":5,\"vehicleID\":601},{\"ETA_Duration\":2000,\"ETA_Minute\":1,\"ETA_Second\":22.001000000000005,\"basicVehicleRole\":16,\"inBoundLaneID\":12,\"msgCount\":6,\"priorityRequestStatus\":4,\"requestID\":5,\"vehicleID\":605},{\"ETA_Duration\":2000,\"ETA_Minute\":1,\"ETA_Second\":22.001000000000005,\"basicVehicleRole\":9,\"inBoundLaneID\":12,\"msgCount\":6,\"priorityRequestStatus\":0,\"requestID\":5,\"vehicleID\":610}],\"sequenceNumber\":4,\"updateCount\":4},\"noOfRequest\":3}";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        ASSERT_TRUE(result);
        string invalid_json = "invalid";
        result = converter.parseJsonString(invalid_json, root);
        ASSERT_FALSE(result);
    }

    TEST_F(test_JsonToJ2735SSMConverter, toJ2735SSM)
    {
        JsonToJ2735SSMConverter converter;
        // Json string refer to: https://github.com/mmitss/mmitss-az/blob/master/src/mrp/priority-request-server/SSM.json
        // Json has three requestors
        string valid_json_str = "{\"MessageType\":\"SSM\",\"SignalStatus\":{\"intersectionID\":1003,\"minuteOfYear\":345239,\"msOfMinute\":54000,\"regionalID\":0,\"requestorInfo\":[{\"ETA_Duration\":2000,\"ETA_Minute\":1,\"ETA_Second\":20.001000000000005,\"basicVehicleRole\":16,\"inBoundApproachID\":8,\"msgCount\":2,\"priorityRequestStatus\":4,\"requestID\":5,\"vehicleID\":601},{\"ETA_Duration\":2000,\"ETA_Minute\":1,\"ETA_Second\":22.001000000000005,\"basicVehicleRole\":16,\"inBoundLaneID\":12,\"msgCount\":6,\"priorityRequestStatus\":4,\"requestID\":5,\"vehicleID\":605},{\"ETA_Duration\":2000,\"ETA_Minute\":1,\"ETA_Second\":22.001000000000005,\"basicVehicleRole\":9,\"inBoundLaneID\":12,\"msgCount\":6,\"priorityRequestStatus\":0,\"requestID\":5,\"vehicleID\":610}],\"sequenceNumber\":4,\"updateCount\":4},\"noOfRequest\":3}";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        ASSERT_TRUE(result);
        auto ssmPtr = std::make_shared<SignalStatusMessage>();
        converter.toJ2735SSM(root, ssmPtr);
        ASSERT_EQ(1, ssmPtr->status.list.count);
        ASSERT_EQ(1003, ssmPtr->status.list.array[0]->id.id);
        ASSERT_EQ(54000, ssmPtr->second);
        ASSERT_EQ(345239, *ssmPtr->timeStamp);
        ASSERT_EQ(3, ssmPtr->status.list.array[0]->sigStatus.list.count);
        ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());
    }

    TEST_F(test_JsonToJ2735SSMConverter, toJ2735SSM_2)
    {
        JsonToJ2735SSMConverter converter;
        // Json has one requestor
        string valid_json_str = "{\"MessageType\":\"SSM\",\"SignalStatus\":{\"intersectionID\":1004,\"minuteOfYear\":345240,\"msOfMinute\":54001,\"regionalID\":0,\"requestorInfo\":[{\"ETA_Duration\":2000,\"ETA_Minute\":1,\"ETA_Second\":20.001000000000005,\"basicVehicleRole\":16,\"inBoundLaneID\":8,\"msgCount\":2,\"priorityRequestStatus\":4,\"requestID\":5,\"vehicleID\":601}],\"sequenceNumber\":4,\"updateCount\":4},\"noOfRequest\":3}";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        ASSERT_TRUE(result);
        auto ssmPtr = std::make_shared<SignalStatusMessage>();
        converter.toJ2735SSM(root, ssmPtr);
        ASSERT_EQ(1, ssmPtr->status.list.count);
        ASSERT_EQ(1004, ssmPtr->status.list.array[0]->id.id);
        ASSERT_EQ(54001, ssmPtr->second);
        ASSERT_EQ(345240, *ssmPtr->timeStamp);
        ASSERT_EQ(1, ssmPtr->status.list.array[0]->sigStatus.list.count);
        ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());
    }

    TEST_F(test_JsonToJ2735SSMConverter, toJ2735SSM_3)
    {
        JsonToJ2735SSMConverter converter;
        // Json has NO requestor
        string valid_json_str = "{\"MessageType\":\"SSM\",\"SignalStatus\":{\"intersectionID\":1004,\"minuteOfYear\":345240,\"msOfMinute\":54001,\"regionalID\":0,\"sequenceNumber\":4,\"updateCount\":4},\"noOfRequest\":3}";
        Json::Value root;
        bool result = converter.parseJsonString(valid_json_str, root);
        ASSERT_TRUE(result);
        auto ssmPtr = std::make_shared<SignalStatusMessage>();
        converter.toJ2735SSM(root, ssmPtr);
        ASSERT_EQ(0, ssmPtr->status.list.array[0]->sigStatus.list.count);
        ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());
    }

    TEST_F(test_JsonToJ2735SSMConverter, encodeSSM)
    {
        JsonToJ2735SSMConverter converter;
        // Json has one requestor
        string valid_json_str = "{\"MessageType\":\"SSM\",\"SignalStatus\":{\"intersectionID\":1004,\"minuteOfYear\":345240,\"msOfMinute\":54001,\"regionalID\":0,\"requestorInfo\":[{\"ETA_Duration\":2000,\"ETA_Minute\":1,\"ETA_Second\":20.001000000000005,\"basicVehicleRole\":16,\"inBoundLaneID\":8,\"msgCount\":2,\"priorityRequestStatus\":4,\"requestID\":5,\"vehicleID\":601}],\"sequenceNumber\":4,\"updateCount\":4},\"noOfRequest\":3}";
        Json::Value root;
        converter.parseJsonString(valid_json_str, root);
        auto ssmPtr = std::make_shared<SignalStatusMessage>();
        converter.toJ2735SSM(root, ssmPtr);
        tmx::messages::SsmEncodedMessage encodedSSM;
        converter.encodeSSM(ssmPtr, encodedSSM);
        string expected_payload_hex = "001e18454498d2f1001007d8054a000004b20a090010000280fa08";
        ASSERT_EQ(expected_payload_hex, encodedSSM.get_payload_str());

        // Json has three requestors
        valid_json_str = "{\"MessageType\":\"SSM\",\"SignalStatus\":{\"intersectionID\":1003,\"minuteOfYear\":345239,\"msOfMinute\":54000,\"regionalID\":0,\"requestorInfo\":[{\"ETA_Duration\":2000,\"ETA_Minute\":1,\"ETA_Second\":20.001000000000005,\"basicVehicleRole\":16,\"inBoundLaneID\":8,\"msgCount\":2,\"priorityRequestStatus\":4,\"requestID\":5,\"vehicleID\":601},{\"ETA_Duration\":2000,\"ETA_Minute\":1,\"ETA_Second\":22.001000000000005,\"basicVehicleRole\":16,\"inBoundLaneID\":12,\"msgCount\":6,\"priorityRequestStatus\":4,\"requestID\":5,\"vehicleID\":605},{\"ETA_Duration\":2000,\"ETA_Minute\":1,\"ETA_Second\":22.001000000000005,\"basicVehicleRole\":9,\"inBoundLaneID\":12,\"msgCount\":6,\"priorityRequestStatus\":0,\"requestID\":5,\"vehicleID\":610}],\"sequenceNumber\":4,\"updateCount\":4},\"noOfRequest\":3}";
        converter.parseJsonString(valid_json_str, root);
        converter.toJ2735SSM(root, ssmPtr);
        converter.encodeSSM(ssmPtr, encodedSSM);
        expected_payload_hex = "001e35454497d2f0001007d6254a000004b20a090010000280fa08a940000097414320030000581f4115280000131028624060000b03e800";
        ASSERT_EQ(expected_payload_hex, encodedSSM.get_payload_str());
        ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SignalStatusMessage, ssmPtr.get());
    }
}