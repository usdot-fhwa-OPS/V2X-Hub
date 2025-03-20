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

TEST(TestIMFNTCIP1218Worker, testSendNTCIP1218ImfMessage) {
    // Test the sendNTCIP1218ImfMessage function
    // Create a mock SNMP client
    std::unique_ptr mockClient = std::make_unique<mock_snmp_client>("", 0, "", "", "", "");
    // Call the function
    std::string message = "30818080011183011384780130201b6bd3420c3bb220a9a79e4c3b32a093bba65e5cf2e3e9a77ee03c48100000b00204343a663a66001021a1d331d33001410c0e49800a086874cc74cc0040454396c396c002022a1cb61cb6001c10c0e49800e08a872d872d800604343a663a66003021a1d331d33000410c0e498002086874cc74ccfd002020602a000000001d0054f3857404e2adefc542027154f886dc04e2827f0008143009000000000748017022f7fff14fd0008127e001008300d000000000e802a03a7ce02715879f419004e2ac4590a00271413f40040618108000000003a40077fffbf5f8a7e0004213ec0081008d4c0010000004944c8804bb2b479f3809760400b50234300040000015a011b804f325d8090027981004d008ccc0010000004fca0f804e3acf990320271c1006cc08d8c00100000050f92d4014529851de80a29040236047d100000000001000e0706fffe11f040000000000400f7fffbf69847b100000000001002dfa86fffe11f84000000000040077fffc09a8";
    unsigned int index = 1;
    snmp_response_obj resp;
    resp.type = snmp_response_obj::response_type::STRING;
    resp.val_string = std::vector<char>(message.begin(), message.end());
    std::string oid = rsu::mib::ntcip1218::rsuIFMPayloadOid +  "." + std::to_string(index);
    EXPECT_CALL( *mockClient, process_snmp_request(oid, request_type::SET , resp) ).Times(1).WillRepeatedly(Return(true));
    
    sendNTCIP1218ImfMessage(mockClient.get(), message, index);
}