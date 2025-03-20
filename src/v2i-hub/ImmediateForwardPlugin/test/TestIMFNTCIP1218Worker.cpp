#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <MockSNMPClient.h>
#include <IMFNTCIP1218Worker.h>

using namespace ImmediateForward;
using namespace unit_test;
using testing::_;
using testing::Action;
using testing::DoDefault;
using testing::Return;
using testing::SetArgReferee;
using testing::Throw;

TEST(TestIMFNTCIP1218Worker, testClearImmediateForwardTable) {
    // Test the clearImmediateForwardTable function
    // Create a mock SNMP client
    std::unique_ptr mockClient = std::make_unique<mock_snmp_client>("", 0, "", "", "", "");
    // Call the function
    snmp_response_obj maxImfsRep;
    maxImfsRep.type = snmp_response_obj::response_type::INTEGER;
    maxImfsRep.val_int = 16;
    EXPECT_CALL( *mockClient, process_snmp_request(rsu::mib::ntcip1218::maxRsuIFMs, request_type::GET , _) ).Times(1).WillRepeatedly(testing::DoAll(
        testing::SetArgReferee<2>(maxImfsRep),
        Return(true)));
    snmp_response_obj deleteRowRep;
    deleteRowRep.type = snmp_response_obj::response_type::INTEGER;
    deleteRowRep.val_int = 6;
    EXPECT_CALL( *mockClient, process_snmp_request(_, request_type::SET , _) ).Times(16).WillRepeatedly(testing::DoAll(
        testing::SetArgReferee<2>(deleteRowRep),
        Return(true)));
    
    clearImmediateForwardTable(mockClient.get());

}

TEST(TestIMFNTCIP1218Worker, testSetRSUMode) {
    // Test the setRSUMode function
    // Create a mock SNMP client
    std::unique_ptr mockClient = std::make_unique<mock_snmp_client>("", 0, "", "", "", "");
    // Call the function
    snmp_response_obj obj;
    obj.type = snmp_response_obj::response_type::INTEGER;
    obj.val_int = 2;
    EXPECT_CALL( *mockClient, process_snmp_request(rsu::mib::ntcip1218::rsuModeOid, request_type::SET , obj) ).Times(1).WillRepeatedly(testing::DoAll(
        testing::SetArgReferee<2>(obj),
        Return(true)));
    
    setRSUMode(mockClient.get(), 2);
}

TEST(TestIMFNTCIP1218Worker, testInitializeImmediateForwardTable) {
    // Test the initializeImmediateForwardTable function
    // Create a mock SNMP client
    std::unique_ptr mockClient = std::make_unique<mock_snmp_client>("", 0, "", "", "", "");
    // Call the function
    std::vector<Message> messages;
    Message message;
    message.tmxType = "SPAT-P";
    message.sendType = "SPAT";
    message.psid = "0x8002";
    message.channel = 183;
    messages.push_back(message);

    std::vector<snmpRequest> requests ;
    EXPECT_CALL( *mockClient, process_snmp_set_requests(_) ).WillOnce(::testing::SaveArg<0>(&requests));
    // Expect these snmpRequests to be called
    // snmpRequest psid{
    //     rsu::mib::ntcip1218::rsuIFMPsidOid + "." + std::to_string(1),
    //     'x',
    //     "8002"
    // };
    // snmpRequest channel{
    //     rsu::mib::ntcip1218::rsuIFMTxChannelOid + "." + std::to_string(1),
    //     'i',
    //     "183"
    // };
    // snmpRequest enable{
    //     rsu::mib::ntcip1218::rsuIFMEnableOid + "." + std::to_string(1),
    //     'i',
    //     "1"
    // };
    // snmpRequest payload{
    //     rsu::mib::ntcip1218::rsuIFMPayloadOid + "." + std::to_string(1),
    //     'x',
    //     "FE"
    // };
    // snmpRequest creatRow{
    //     rsu::mib::ntcip1218::rsuIFMStatusOid + "." + std::to_string(1),
    //     'i',
    //     "4"
    // };
    // snmpRequest priority{
    //     rsu::mib::ntcip1218::rsuIFMPriorityOid + "." + std::to_string(1),
    //     'i',
    //     "6"
    // };

    // snmpRequest options{
    //     rsu::mib::ntcip1218::rsuIFMOptionsOid + "." + std::to_string(1),
    //     'x',
    //     "01"
    // };
    initializeImmediateForwardTable(mockClient.get(), messages);
    EXPECT_EQ(requests.size(), 7);
    EXPECT_EQ(requests[0].oid, rsu::mib::ntcip1218::rsuIFMPsidOid + "." + std::to_string(1));
    EXPECT_EQ(requests[0].type, 'x');
    EXPECT_EQ(requests[0].value, "8002");
    EXPECT_EQ(requests[1].oid, rsu::mib::ntcip1218::rsuIFMTxChannelOid + "." + std::to_string(1));
    EXPECT_EQ(requests[1].type, 'i');
    EXPECT_EQ(requests[1].value, "183");
    EXPECT_EQ(requests[2].oid, rsu::mib::ntcip1218::rsuIFMPayloadOid + "." + std::to_string(1));
    EXPECT_EQ(requests[2].type, 'x');
    EXPECT_EQ(requests[2].value, "FE");
    EXPECT_EQ(requests[3].oid, rsu::mib::ntcip1218::rsuIFMEnableOid + "." + std::to_string(1));
    EXPECT_EQ(requests[3].type, 'i');
    EXPECT_EQ(requests[3].value, "1");
    EXPECT_EQ(requests[4].oid, rsu::mib::ntcip1218::rsuIFMStatusOid + "." + std::to_string(1));
    EXPECT_EQ(requests[4].type, 'i');
    EXPECT_EQ(requests[4].value, "4");
    EXPECT_EQ(requests[5].oid, rsu::mib::ntcip1218::rsuIFMPriorityOid + "." + std::to_string(1));
    EXPECT_EQ(requests[5].type, 'i');
    EXPECT_EQ(requests[5].value, "6");
    EXPECT_EQ(requests[6].oid, rsu::mib::ntcip1218::rsuIFMOptionsOid + "." + std::to_string(1));
    EXPECT_EQ(requests[6].type, 'x');
    EXPECT_EQ(requests[6].value, "01");

}