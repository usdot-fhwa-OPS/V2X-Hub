//============================================================================
// Name        : J2735MessageTest.cpp
// Description : Unit tests for the J2735 Message library.
//============================================================================

#include <boost/any.hpp>
#include <gtest/gtest.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <tmx/j2735_messages/testMessage03.hpp>
#include <tmx/messages/message_document.hpp>
#include <array>
#include <vector>
#include <iostream>
#include <chrono>
#include <sstream>
#include <cassert>
#include <stol-j2735-201603-carma/jer_encoder.h>

using namespace std;
using namespace battelle::attributes;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::messages::j2735;

namespace unit_test {

TEST(J2735MessageTest, EncodeMobilityOperation)
{	
	// Allocate a C-style struct using j2735::j2735_create which includes custom delete (see SaeJ2735Traits.hpp).
	auto message = j2735::j2735_create<tsm3Traits>(); 
	
	/**
	 * Populate MobilityHeader 
	 */
	int failed = 1;
	
	std::string hostStaticId_str = "host_id";
	failed = OCTET_STRING_fromString( &(message->header.hostStaticId), hostStaticId_str.c_str() );
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);
	std::string targetStaticId_str = "targer_id";
	failed = OCTET_STRING_fromString( &message->header.targetStaticId, targetStaticId_str.c_str() );
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	std::string bsmId_str = "bsm_idXX";
	failed = OCTET_STRING_fromString( &message->header.hostBSMId, bsmId_str.c_str() );
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	std::string planId_str = "00000000-0000-0000-0000-000000000000";
	failed = OCTET_STRING_fromString( &message->header.planId, planId_str.c_str() );
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	std::string timestamp_str = std::to_string(1784819631870201847);
	failed = OCTET_STRING_fromString( &message->header.timestamp, timestamp_str.c_str());
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);
	

	/**
	 * Populate MobilityOperation Body 
	 */
	
	std::string operationParams_str = "traffic_control_id: traffic_control_id, acknowledgement: true, reason: optional reason text";
	failed = OCTET_STRING_fromString( &message->body.operationParams, operationParams_str.c_str());
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	
	std::string strategy_str = "carma3/Geofence_Acknowledgement";
	failed = OCTET_STRING_fromString( &message->body.strategy, strategy_str.c_str());
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	tmx::messages::tsm3EncodedMessage tsm3EncodeMessage;
	tmx::messages::tsm3Message  _tsm3Message(message);
	tmx::messages::MessageFrameMessage frame_msg(_tsm3Message.get_j2735_data());
	tsm3EncodeMessage.set_data(TmxJ2735EncodedMessage<TestMessage03>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
		
	// Get encode message as hex string
	tmx::byte_stream bytes = tsm3EncodeMessage.get_payload_bytes();
	std::string hex_str = tmx::byte_stream_encode(bytes);
	// Verify UPER encoding matches expected value
	std::string expected_hex_str = "00f380b32e8dfcfa5fd391fa61e59f2f2bfa7262e7b6fe9c962c3060c183060c182d60c18305ac183060b583060c16b060c183060c183060c18316ee1a3862e5b3362e1bb064c18b868dddc78796dc2cd7c7cbbf365dd8f2df838f5eedfdf665c99f2edcbbba0b3d3961cd9b4e3bf8f7eee9cb7ecbfa723a41d3961cd9b4e3bf8f7eee9cb7ecbfa722c41871ebddbfbeccb933e5db977747483a72eb95620e5970f3dfb9d20dfc3a69dfbb0ec41cb2e1e7bf720e997c74";
	EXPECT_EQ(hex_str, expected_hex_str);
}

TEST(J2735MessageTest, EncodeMobilityRequest)
{	
	// Allocate a C-style struct and manage it with shared_ptr and a custom deleter.
	auto message = j2735::j2735_create<tsm0Traits>();
	
	/**
	 * Populate MobilityHeader 
	 */
	int failed = 1;
	
	std::string hostStaticId_str = "host_id";
	failed = OCTET_STRING_fromString( &(message->header.hostStaticId), hostStaticId_str.c_str() );
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);
	std::string targetStaticId_str = "targer_id";
	failed = OCTET_STRING_fromString( &message->header.targetStaticId, targetStaticId_str.c_str() );
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	std::string bsmId_str = "bsm_idXX";
	failed = OCTET_STRING_fromString( &message->header.hostBSMId, bsmId_str.c_str() );
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	std::string planId_str = "00000000-0000-0000-0000-000000000000";
	failed = OCTET_STRING_fromString( &message->header.planId, planId_str.c_str() );
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	std::string timestamp_str = std::to_string(1784819631870201847);
	failed = OCTET_STRING_fromString( &message->header.timestamp, timestamp_str.c_str());
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	/**
	 * Populate MobilityOperation Body 
	 */
	
	std::string strategyParams_str = "traffic_control_id: traffic_control_id, acknowledgement: true, reason: optional reason text";
	failed = OCTET_STRING_fromString( &message->body.strategyParams, strategyParams_str.c_str());
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	
	std::string strategy_str = "carma3/Geofence_Acknowledgement";
	failed = OCTET_STRING_fromString( &message->body.strategy, strategy_str.c_str());
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	message->body.urgency = 1;
	message->body.planType = 0;
	message->body.location.ecefX = 1;
	message->body.location.ecefY = 1;
	message->body.location.ecefZ = 1;
	failed = OCTET_STRING_fromString( &message->body.location.timestamp, timestamp_str.c_str());
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);
	
	std::string expiration_str = std::to_string(1784819631875000000);
	message->body.expiration = AllocAsn<MobilityTimestamp_t>();
	failed = OCTET_STRING_fromString( message->body.expiration, expiration_str.c_str());
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);
	

	MobilityECEFOffset_t offset ;
	offset.offsetX = 1;
	offset.offsetY = 1;
	offset.offsetZ = 1;
	ASN_SEQUENCE_ADD(&message->body.trajectory->list.array, &offset);
	ASN_SEQUENCE_ADD(&message->body.trajectory->list.array, &offset);

	message->body.trajectoryStart = AllocAsn<MobilityLocation>();
	message->body.trajectoryStart->ecefX = 1;
	message->body.trajectoryStart->ecefY = 1;
	message->body.trajectoryStart->ecefZ = 1;
	failed = OCTET_STRING_fromString( &message->body.trajectoryStart->timestamp, timestamp_str.c_str());
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);
		
	tmx::messages::tsm0EncodedMessage tsm0EncodeMessage;
	tmx::messages::tsm0Message  _tsm0Message(message);
	tmx::messages::MessageFrameMessage frame_msg(_tsm0Message.get_j2735_data());
	tsm0EncodeMessage.set_data(TmxJ2735EncodedMessage<TestMessage00>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
		
	// Get encode message as hex string
	tmx::byte_stream bytes = tsm0EncodeMessage.get_payload_bytes();
	std::string hex_str = tmx::byte_stream_encode(bytes);
	// Verify UPER encoding matches expected value
	std::string expected_hex_str = "00f080ff2e8dfcfa5fd391fa61e59f2f2bfa7262e7b6fe9c962c3060c183060c182d60c18305ac183060b583060c16b060c183060c183060c18316ee1a3862e5b3362e1bb064c18b868debb8f0f2db859af8f977e6cbbb1e5bf071ebddbfbeccb933e5db9777400029832a0d5306541aa60ca83562ddc3470c5cb666c5c3760c983170d1b8b3d3961cd9b4e3bf8f7eee9cb7ecbfa723a41d3961cd9b4e3bf8f7eee9cb7ecbfa722c41871ebddbfbeccb933e5db977747483a72eb95620e5970f3dfb9d20dfc3a69dfbb0ec41cb2e1e7bf720e997c744c19506a9832a0d5306541ab16ee1a3862e5b3362e1bb064c18b868dd8b770d1c3172d99b170ddab060c1830600";
	EXPECT_EQ(hex_str, expected_hex_str);
}

TEST(J2735MessageTest, EncodeMobilityResponse)
{	
	// Allocate a C-style struct and manage it with shared_ptr and a custom deleter.
	auto message = j2735::j2735_create<tsm1Traits>();
	/**
	 * Populate MobilityHeader 
	 */
	int failed = 1;
	
	std::string hostStaticId_str = "host_id";
	failed = OCTET_STRING_fromString( &(message->header.hostStaticId), hostStaticId_str.c_str() );
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);
	std::string targetStaticId_str = "targer_id";
	failed = OCTET_STRING_fromString( &message->header.targetStaticId, targetStaticId_str.c_str() );
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	std::string bsmId_str = "bsm_idXX";
	failed = OCTET_STRING_fromString( &message->header.hostBSMId, bsmId_str.c_str() );
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	std::string planId_str = "00000000-0000-0000-0000-000000000000";
	failed = OCTET_STRING_fromString( &message->header.planId, planId_str.c_str() );
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	std::string timestamp_str = std::to_string(1784819631870201847);
	failed = OCTET_STRING_fromString( &message->header.timestamp, timestamp_str.c_str());
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	/**
	 * Populate MobilityResponse Body 
	 */
	message->body.isAccepted = true;
	// Int from 0-1000. See ASN.1 definition
	message->body.urgency = 1;
	//Enumeration between 0-13. See ASN.1 definition
	message->body.planType = 3; // PlanType: joinPlatoonAtRear
	message->body.reason = AllocAsn<MobilityReason_t>();
	// Enumeration between 0-8. See ASN.1 definition
	*message->body.reason = 5; // Reason: OtherwiseEngaged
	message->body.repeat = AllocAsn<MobilityRepeat_t>();
	// Enumeration between 0-2. See ASN.1 definition
	*message->body.repeat = 2; // Repeat: neverRequestAgain

	asn_fprint(stdout, &asn_DEF_TestMessage01, message.get());

	tmx::messages::tsm1EncodedMessage tsm1EncodeMessage;
	tmx::messages::tsm1Message  _tsm1Message(message);
	tmx::messages::MessageFrameMessage frame_msg(_tsm1Message.get_j2735_data());
	tsm1EncodeMessage.set_data(TmxJ2735EncodedMessage<TestMessage01>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
		
	// Get encode message as hex string
	tmx::byte_stream bytes = tsm1EncodeMessage.get_payload_bytes();
	std::string hex_str = tmx::byte_stream_encode(bytes);
	// Verify UPER encoding matches expected value
	std::string expected_hex_str = "00f14a2e8dfcfa5fd391fa61e59f2f2bfa7262e7b6fe9c962c3060c183060c182d60c18305ac183060b583060c16b060c183060c183060c18316ee1a3862e5b3362e1bb064c18b868df00632a0";
	EXPECT_EQ(hex_str, expected_hex_str);

}

TEST(J2735MessageTest, EncodeBasicSafetyMessage)
{	
	// Allocate a C-style struct and manage it with shared_ptr and a custom deleter.
	auto message = j2735::j2735_create<BsmTraits>();
	/**
	 * Populate BSMcoreData 
	 */
	message->coreData.msgCnt = 1;
	// TempId is octet string and can be populated from buffer
	char  my_bytes_id[4] = {(char)1, (char)12, (char)12, (char)10};
	bool failed = OCTET_STRING_fromBuf(&message->coreData.id, my_bytes_id, sizeof(my_bytes_id));
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	message->coreData.secMark = 1023;
	message->coreData.lat = 38954961;
	message->coreData.Long = -77149303;
	message->coreData.elev = 72;
	message->coreData.speed = 100;
	message->coreData.heading = 12;
	message->coreData.angle = 10;
	message->coreData.transmission = 0;  // allow 0...7

	//position accuracy
	message->coreData.accuracy.orientation= 100;
	message->coreData.accuracy.semiMajor = 200;
	message->coreData.accuracy.semiMinor = 200;

	//Acceleration set
	message->coreData.accelSet.lat = 100;
	message->coreData.accelSet.Long = 300;
	message->coreData.accelSet.vert = 100;
	message->coreData.accelSet.yaw = 0;

	//populate brakes
	message->coreData.brakes.abs = 1; // allow 0,1,2,3
	message->coreData.brakes.scs = 1; // allow 0,1,2,3
	message->coreData.brakes.traction = 1; // allow 0,1,2,3
	message->coreData.brakes.brakeBoost = 1; // allow 0,1,2
	message->coreData.brakes.auxBrakes = 1; // allow 0,1,2,3
	// Allocate memory for wheelBrakes BIT_STRING. Otherwise Asan gives error :
	// ERROR: AddressSanitizer: attempting free on address which was not malloc()-ed
	uint8_t  *my_bytes_brakes = static_cast<uint8_t *>(calloc(1, sizeof(uint8_t)));
	*my_bytes_brakes = 8;
	message->coreData.brakes.wheelBrakes.buf = my_bytes_brakes; // allow 0,1,2,3,4
	message->coreData.brakes.wheelBrakes.size = 1; // allow 0,1,2,3,4	
	message->coreData.brakes.wheelBrakes.bits_unused = 3; // allow 0,1,2,3,4	

	//vehicle size
	message->coreData.size.length = 500;
	message->coreData.size.width = 300;

	asn_fprint(stdout, &asn_DEF_BasicSafetyMessage, message.get());

	tmx::messages::BsmEncodedMessage bsmEncodeMessage;
	tmx::messages::BsmMessage  _bsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_bsmMessage.get_j2735_data());
	bsmEncodeMessage.set_data(TmxJ2735EncodedMessage<BasicSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
		
	
	// Get encode message as hex string
	tmx::byte_stream bytes = bsmEncodeMessage.get_payload_bytes();
	std::string hex_str = tmx::byte_stream_encode(bytes);
	// Verify UPER encoding matches expected value
	std::string expected_hex_str = "001425004043030280ffdbfba868b3584ec40824646400320032000c888fc834e37fff0aaa960fa0";
	EXPECT_EQ(hex_str, expected_hex_str);
}



TEST(J2735MessageTest, EncodeBasicSafetyMessagePartII)
{	
// Allocate a C-style struct and manage it with shared_ptr and a custom deleter.
	auto message = j2735::j2735_create<BsmTraits>();	
	/**
	 * Populate BSMcoreData 
	 */
	message->coreData.msgCnt = 1;
	// TempId is octet string and can be populated from buffer
	char  my_bytes_id[4] = {(char)1, (char)12, (char)12, (char)10};
	bool failed = OCTET_STRING_fromBuf(&message->coreData.id, my_bytes_id, sizeof(my_bytes_id));
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	message->coreData.secMark = 1023;
	message->coreData.lat = 38954961;
	message->coreData.Long = -77149303;
	message->coreData.elev = 72;
	message->coreData.speed = 100;
	message->coreData.heading = 12;
	message->coreData.angle = 10;
	message->coreData.transmission = 0;  // allow 0...7

	//position accuracy
	message->coreData.accuracy.orientation= 100;
	message->coreData.accuracy.semiMajor = 200;
	message->coreData.accuracy.semiMinor = 200;

	//Acceleration set
	message->coreData.accelSet.lat = 100;
	message->coreData.accelSet.Long = 300;
	message->coreData.accelSet.vert = 100;
	message->coreData.accelSet.yaw = 0;

	//populate brakes
	message->coreData.brakes.abs = 1; // allow 0,1,2,3
	message->coreData.brakes.scs = 1; // allow 0,1,2,3
	message->coreData.brakes.traction = 1; // allow 0,1,2,3
	message->coreData.brakes.brakeBoost = 1; // allow 0,1,2
	message->coreData.brakes.auxBrakes = 1; // allow 0,1,2,3
	// Allocate memory for wheelBrakes BIT_STRING. Otherwise Asan gives error :
	// ERROR: AddressSanitizer: attempting free on address which was not malloc()-ed
	uint8_t  *my_bytes_brakes = static_cast<uint8_t *>(calloc(1, sizeof(uint8_t)));
	*my_bytes_brakes = 8;
	message->coreData.brakes.wheelBrakes.buf = my_bytes_brakes; // allow 0,1,2,3,4
	message->coreData.brakes.wheelBrakes.size =1; // allow 0,1,2,3,4	
	message->coreData.brakes.wheelBrakes.bits_unused = 3; // allow 0,1,2,3,4	

	//vehicle size
	message->coreData.size.length = 500;
	message->coreData.size.width = 300;

	// Allocate optional BSM partII
	message->partII = AllocAsn<BasicSafetyMessage::BasicSafetyMessage__partII>();
	auto partIICnt = AllocAsn<BSMpartIIExtension_t>();
	partIICnt->partII_Id = 1;
	partIICnt->partII_Value.present = BSMpartIIExtension__partII_Value_PR_SpecialVehicleExtensions;

	partIICnt->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts = AllocAsn<EmergencyDetails_t> ();
	partIICnt->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->lightsUse = LightbarInUse_inUse;
	partIICnt->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->responseType = AllocAsn<ResponseType_t>();
	*partIICnt->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->responseType = ResponseType_emergency;
	partIICnt->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->sirenUse = SirenInUse_inUse;	
	
    asn_sequence_add(&message->partII->list.array, partIICnt);
	// BSM regional extension
    message->regional = AllocAsn<BasicSafetyMessage::BasicSafetyMessage__regional>();
    auto reg_bsm = AllocAsn<Reg_BasicSafetyMessage_t>();
    reg_bsm->regionId = 128;
    reg_bsm->regExtValue.present = Reg_BasicSafetyMessage__regExtValue_PR_BasicSafetyMessage_addGrpCarma;

    reg_bsm->regExtValue.choice.BasicSafetyMessage_addGrpCarma.routeDestinationPoints = AllocAsn<BasicSafetyMessage_addGrpCarma::BasicSafetyMessage_addGrpCarma__routeDestinationPoints>();
    auto point = AllocAsn<Position3D_t>();
	auto dummy_lat = 12;
	auto dummy_long = 1312;
    point->lat = dummy_lat;
    point->Long = dummy_long;
    asn_sequence_add(&reg_bsm->regExtValue.choice.BasicSafetyMessage_addGrpCarma.routeDestinationPoints->list.array, point);
    auto point2 = AllocAsn<Position3D_t>();
    point2->lat = dummy_lat + 1000;
    point2->Long = dummy_long + 1000;
    asn_sequence_add(&reg_bsm->regExtValue.choice.BasicSafetyMessage_addGrpCarma.routeDestinationPoints->list.array, point2);

    asn_sequence_add(&message->regional->list.array, reg_bsm);

	asn_fprint(stdout, &asn_DEF_BasicSafetyMessage, message.get());
	//Encode BSM
	tmx::messages::BsmEncodedMessage bsmEncodeMessage;
	tmx::messages::BsmMessage  _bsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_bsmMessage.get_j2735_data());
	bsmEncodeMessage.set_data(TmxJ2735EncodedMessage<BasicSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
	
	std::string expectedBSMEncHex = "00143d604043030280ffdbfba868b3584ec40824646400320032000c888fc834e37fff0aaa960fa0040d082408801148d693a431ad275c7c6b49d9e8d693b60e";
	EXPECT_EQ(expectedBSMEncHex, bsmEncodeMessage.get_payload_str());

}

TEST(J2735MessageTest, DecodeBasicSafetyMessagePartII){
	J2735MessageFactory factory;
	std::string hexString = "00143d604043030280ffdbfba868b3584ec40824646400320032000c888fc834e37fff0aaa960fa0040d082408801148d693a431ad275c7c6b49d9e8d693b60e";
	tmx::byte_stream bytes = tmx::byte_stream_decode(hexString);
	std::shared_ptr<MessageFrameEncodedMessage> msg = std::shared_ptr<MessageFrameEncodedMessage>(static_cast<MessageFrameEncodedMessage*>(factory.NewMessage(bytes)));
	auto decoded_msg = msg->decode_j2735_message();
	std::string expected_json_message = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<MessageFrame><messageId>20</messageId><value><BasicSafetyMessage><coreData><msgCnt>1</msgCnt><id>010C0C0A</id><secMark>1023</secMark><lat>38954961</lat><long>-77149303</long><elev>72</elev><accuracy><semiMajor>200</semiMajor><semiMinor>200</semiMinor><orientation>100</orientation></accuracy><transmission><neutral/></transmission><speed>100</speed><heading>12</heading><angle>10</angle><accelSet><long>300</long><lat>100</lat><vert>100</vert><yaw>0</yaw></accelSet><brakes><wheelBrakes>00001</wheelBrakes><traction><off/></traction><abs><off/></abs><scs><off/></scs><brakeBoost><off/></brakeBoost><auxBrakes><off/></auxBrakes></brakes><size><width>300</width><length>500</length></size></coreData><partII><BSMpartIIExtension><partII-Id>1</partII-Id><partII-Value><SpecialVehicleExtensions><vehicleAlerts><doNotUse>0</doNotUse><sirenUse><inUse/></sirenUse><lightsUse><inUse/></lightsUse><multi><unavailable/></multi><responseType><emergency/></responseType></vehicleAlerts></SpecialVehicleExtensions></partII-Value></BSMpartIIExtension></partII><regional><Reg-BasicSafetyMessage><regionId>128</regionId><regExtValue><BasicSafetyMessage-addGrpCarma><routeDestinationPoints><Position3D-addGrpCarma><lat>12</lat><long>1312</long></Position3D-addGrpCarma><Position3D-addGrpCarma><lat>1012</lat><long>2312</long></Position3D-addGrpCarma></routeDestinationPoints></BasicSafetyMessage-addGrpCarma></regExtValue></Reg-BasicSafetyMessage></regional></BasicSafetyMessage></value></MessageFrame>";
	EXPECT_EQ(decoded_msg.to_string(),expected_json_message);
}

TEST(J2735MessageTest, EncodePersonalSafetyMessage){
	string psm="<PersonalSafetyMessage><basicType><aPEDESTRIAN/></basicType><secMark>109</secMark><msgCnt>0</msgCnt><id>115eadf0</id><position><lat>389549376</lat><long>-771491840</long></position><accuracy><semiMajor>255</semiMajor><semiMinor>255</semiMinor><orientation>65535</orientation></accuracy><speed>0</speed><heading>16010</heading><pathHistory><crumbData><PathHistoryPoint><latOffset>0</latOffset><lonOffset>0</lonOffset><elevationOffset>0</elevationOffset><timeOffset>1</timeOffset></PathHistoryPoint></crumbData></pathHistory></PersonalSafetyMessage>";
	std::stringstream ss;
	PsmMessage psmmessage;
	PsmEncodedMessage psmENC;
	tmx::message_container_type container;
	ss<<psm;
	container.load<XML>(ss);
	psmmessage.set_contents(container.get_storage().get_tree());
	psmENC.encode_j2735_message(psmmessage);
	
	// Get UPER hex
	tmx::byte_stream bytes = psmENC.get_payload_bytes();
	std::string hexString =  tmx::byte_stream_encode(bytes);
	std::string expectedHexString = "00202320000200da00457ab7c04cdcf6403d4dc9ffffffffff0003e8a0008000200008000000";
	EXPECT_EQ(expectedHexString, hexString);

}

TEST(J2735MessageTest, EncodeTrafficControlRequest){
	string tsm4str="<TestMessage04><body><tcrV01><reqid>C7C9A13FE6AC464E</reqid><reqseq>0</reqseq><scale>0</scale><bounds><TrafficControlBounds><oldest>27493419</oldest><reflon>-818349472</reflon><reflat>281118677</reflat><offsets><OffsetPoint><deltax>376</deltax><deltay>0</deltay></OffsetPoint><OffsetPoint><deltax>376</deltax><deltay>1320</deltay></OffsetPoint><OffsetPoint><deltax>0</deltax><deltay>1320</deltay></OffsetPoint></offsets></TrafficControlBounds></bounds></tcrV01> </body></TestMessage04>";
	std::stringstream ss;
	tsm4Message tsm4msg;
	tsm4EncodedMessage tsm4Enc;
	tmx::message_container_type container;
	ss<<tsm4str;
	container.load<XML>(ss);
	tsm4msg.set_contents(container.get_storage().get_tree());
	tsm4Enc.encode_j2735_message(tsm4msg);
	
	// Get UPER HEX
	tmx::byte_stream bytes = tsm4Enc.get_payload_bytes();
	std::string hexString =  tmx::byte_stream_encode(bytes);
	std::string expectedHexString = "00f42538f93427fcd588c9c00c0000001a3842b3a82cc5f8ccce1ab02f1000102f10a5100010a500";
	EXPECT_EQ(expectedHexString, hexString);
}

TEST(J2735MessageTest, EncodeTrafficControlMessage){
	//Has <refwidth> tag in TCM
	string tsm5str="<TestMessage05><body><tcmV01><reqid>30642B129B984162</reqid><reqseq>0</reqseq><msgtot>9</msgtot><msgnum>9</msgnum><id>0034b8d88d084ffdaf23837926031658</id><updated>0</updated><package><label>workzone-laneclosed</label><tcids><Id128b>0034b8d88d084ffdaf23837926031658</Id128b></tcids></package><params><vclasses><micromobile/><motorcycle/><passenger-car/><light-truck-van/><bus/><two-axle-six-tire-single-unit-truck/><three-axle-single-unit-truck/><four-or-more-axle-single-unit-truck/><four-or-fewer-axle-single-trailer-truck/><five-axle-single-trailer-truck/><six-or-more-axle-single-trailer-truck/><five-or-fewer-axle-multi-trailer-truck/><six-axle-multi-trailer-truck/><seven-or-more-axle-multi-trailer-truck/></vclasses><schedule><start>27506547</start><end>153722867280912</end><dow>1111111</dow></schedule><regulatory><true/></regulatory><detail><closed><taperleft/></closed></detail></params><geometry><proj>epsg:3785</proj><datum>WGS84</datum><reftime>27506547</reftime><reflon>-818331529</reflon><reflat>281182119</reflat><refelv>0</refelv><refwidth>424</refwidth><heading>3403</heading><nodes><PathNode><x>0</x><y>0</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>721</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-204</x><y>722</y><width>2</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>-2</width></PathNode><PathNode><x>-203</x><y>721</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-13</x><y>46</y><width>0</width></PathNode></nodes></geometry></tcmV01></body></TestMessage05>";
	std::stringstream ss;
	tsm5Message tsm5msg;
	tsm5EncodedMessage tsm5Enc;
	tmx::message_container_type container;
	ss<<tsm5str;
	container.load<XML>(ss);
	tsm5msg.set_contents(container.get_storage().get_tree());
	tsm5Enc.encode_j2735_message(tsm5msg);
	tmx::byte_stream bytes = tsm5Enc.get_payload_bytes();
	std::string hexString =  tmx::byte_stream_encode(bytes);
	std::string expectedHexString = "00f580d83cc190ac4a6e610588000024002400d2e36234213ff6bc8e0de4980c5960000000000002977dfcb5fadfbb2add987765c7b37f3cb9000034b8d88d084ffdaf238379260316580d0c41461c824a2cc34e3d0c000001a3b7738bcf64e5ec1007ff1549cbc39e774cdbb86a2d78f4dc34000001a3b7733a8312768cced14e200006a352c3d80008000805fcd60b4a017f3582d2805fcd60b4a017f3582d1805fcd60b4a017f3582d2805fcd20b4a097f3582d2805fcd60b49f97f3582d1805fcd60b4a017f3582d2805fcd60b4a017f3582d2805ffce00ba000";
	EXPECT_EQ(expectedHexString, hexString);
			
}

TEST(J2735MessageTest, EncodeTrafficControlMessageWithoutRefwidth) {
	//Has <refwidth> tag in TCM
	std::string tsm5str="<TestMessage05><body><tcmV01><reqid>D0E0C6E650394C06</reqid><reqseq>0</reqseq><msgtot>1</msgtot><msgnum>1</msgnum><id>002740591d261d2e99e477df0a82db26</id><updated>0</updated><package><label>workzone</label><tcids><Id128b>002740591d261d2e99e477df0a82db26</Id128b></tcids></package><params><vclasses><micromobile/><motorcycle/><passenger-car/><light-truck-van/><bus/><two-axle-six-tire-single-unit-truck/><three-axle-single-unit-truck/><four-or-more-axle-single-unit-truck/><four-or-fewer-axle-single-trailer-truck/><five-axle-single-trailer-truck/><six-or-more-axle-single-trailer-truck/><five-or-fewer-axle-multi-trailer-truck/><six-axle-multi-trailer-truck/><seven-or-more-axle-multi-trailer-truck/></vclasses><schedule><start>27777312</start><end>153722867280912</end><dow>1111111</dow></schedule><regulatory><true/></regulatory><detail><closed><taperleft/></closed></detail></params><geometry><proj>epsg:3785</proj><datum>WGS84</datum><reftime>27777312</reftime><reflon>-771483519</reflon><reflat>389549109</reflat><refelv>0</refelv><heading>3312</heading><nodes><PathNode><x>1</x><y>0</y><width>0</width></PathNode><PathNode><x>-1498</x><y>-26</y><width>2</width></PathNode><PathNode><x>-1497</x><y>45</y><width>7</width></PathNode><PathNode><x>-1497</x><y>91</y><width>11</width></PathNode><PathNode><x>-370</x><y>34</y><width>2</width></PathNode></nodes></geometry></tcmV01></body></TestMessage05>";
	std::stringstream ss;
	tsm5Message tsm5msg;
	tsm5EncodedMessage tsm5Enc;
	tmx::message_container_type container;
	ss<<tsm5str;
	container.load<XML>(ss);
	tsm5msg.set_contents(container.get_storage().get_tree());
	tsm5Enc.encode_j2735_message(tsm5msg);
	tmx::byte_stream bytes = tsm5Enc.get_payload_bytes();
	std::string hexString =  tmx::byte_stream_encode(bytes);
	std::string expectedHexString = "00f580923f43831b9940e530180000040004009d0164749874ba6791df7c2a0b6c980000000000023f7dfcb5fadfbb280004e80b23a4c3a5d33c8efbe1505b64c1a18828c39049459869c7a180000034fb241179ec9cbd8200ffe2a1397873cee99b770d45af1e9b8680000034fb2407a9bd5013373d4d440033c01180018000805e899ff9a097a27802d875e89e016e2d7e8e802282";
	EXPECT_EQ(expectedHexString, hexString);
}

TEST (J2735MessageTest, EncodeSrm)
{
	// Allocate a C-style struct and manage it with shared_ptr and a custom deleter.
	auto message = j2735::j2735_create<SrmTraits>();

	message->second = 12;
	message->requestor.id.present = VehicleID_PR_entityID;
		// TempId is octet string and can be populated from buffer
	char  my_bytes_id[4] = {(char)1, (char)12, (char)12, (char)10};
	bool failed = OCTET_STRING_fromBuf(&message->requestor.id.choice.entityID, my_bytes_id, sizeof(my_bytes_id));
	// If operation fails, unit test is no longer valid
	ASSERT_EQ(failed, 0);

	message->requestor.position = AllocAsn<RequestorPositionVector_t>();
	#if SAEJ2735_SPEC < 2020
	DSRC_Angle_t *heading_angle = AllocAsn<DSRC_Angle_t>();
	#else
	auto *heading_angle = AllocAsn<Common_Angle_t>();
	#endif
	*heading_angle = 123;
	message->requestor.position->heading = heading_angle;
	#if SAEJ2735_SPEC < 2020
	message->requestor.position->position.elevation = AllocAsn<DSRC_Elevation_t>();
	#else
	message->requestor.position->position.elevation = AllocAsn<Common_Elevation_t>();
	#endif
	*message->requestor.position->position.elevation = 12;
	message->requestor.position->position.lat = 3712333;
	message->requestor.position->position.Long = 8012333;
	TransmissionAndSpeed_t *speed = AllocAsn<TransmissionAndSpeed_t>();
	speed->speed = 10;
	speed->transmisson = 7;
	message->requestor.position->speed = speed;

	SignalRequestList_t *requests = AllocAsn<SignalRequestList_t>();
	//First: Request Package
	SignalRequestPackage_t *request_package = AllocAsn<SignalRequestPackage_t>();
	{
		// Seperate context so that each request package has its own allocated values 
		MinuteOfTheYear_t *min = AllocAsn<MinuteOfTheYear_t>();
		*min = 123;
		request_package->minute = min;
		DSecond_t *duration = AllocAsn<DSecond_t>();
		*duration = 122;
		request_package->duration = duration;
		DSecond_t *second = AllocAsn<DSecond_t>();
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
	SignalRequestPackage_t *request_package_2 = AllocAsn<SignalRequestPackage_t>();
	{
		MinuteOfTheYear_t *min = AllocAsn<MinuteOfTheYear_t>();
		*min = 123;
		request_package_2->minute = min;
		DSecond_t *duration = AllocAsn<DSecond_t>();
		*duration = 122;
		request_package_2->duration = duration;
		DSecond_t *second = AllocAsn<DSecond_t>();
		*second = 1212;
		request_package_2->second = second;
	}
	request_package_2->request.id.id = 2333;
	request_package_2->request.requestID = 2;
	request_package_2->request.requestType = 1;

	request_package_2->request.inBoundLane.present =IntersectionAccessPoint_PR_approach;
	request_package_2->request.inBoundLane.choice.approach = 1;
	asn_sequence_add(&requests->list.array, request_package_2);
	message->requests = requests;

	asn_fprint(stdout, &asn_DEF_SignalRequestMessage , message.get());
	tmx::messages::SrmEncodedMessage srmEncodeMessage;
	tmx::messages::SrmMessage _srmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_srmMessage.get_j2735_data());
	srmEncodeMessage.set_data(TmxJ2735EncodedMessage<SignalRequestMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));	

	std::string expectedSRMEncHex = "001d301000605c0098c020008003d825e003d380247408910007b04bc007a20004303029a6bbb1c9ad7882858201801ef80280";
	EXPECT_EQ(expectedSRMEncHex, srmEncodeMessage.get_payload_str());	
}

TEST(J2735MessageTest, EncodeTravelerInformation){
	#if SAEJ2735_SPEC >= 2024
	//2024 TIM message
	string timStr = R"(
	<TravelerInformation>
		<msgCnt>1</msgCnt>
		<packetID>0000000000087FAA72</packetID>
		<dataFrames>
			<TravelerDataFrame>
			<doNotUse1>0</doNotUse1>
			<frameType>
				<roadSignage/>
			</frameType>
			<msgId>
				<roadSignID>
				<position>
					<lat>281185423</lat>
					<long>-818311882</long>
					<elevation>220</elevation>
				</position>
				<viewAngle>1100000000000001</viewAngle>
				<mutcdCode>
					<maintenance/>
				</mutcdCode>
				</roadSignID>
			</msgId>
			<startYear>2025</startYear>
			<startTime>181181</startTime>
			<durationTime>5760</durationTime>
			<priority>5</priority>
			<doNotUse2>0</doNotUse2>
			<regions>
				<GeographicalPath>
				<anchor>
					<lat>281185423</lat>
					<long>-818311882</long>
					<elevation>220</elevation>
				</anchor>
				<laneWidth>366</laneWidth>
				<directionality>
					<forward/>
				</directionality>
				<closedPath>
					<true/>
				</closedPath>
				<direction>1100000000000001</direction>
				<description>
					<path>
					<offset>
						<xy>
						<nodes>
							<NodeXY>
							<delta>
								<node-XY3>
								<x>277</x>
								<y>-1815</y>
								</node-XY3>
							</delta>
							</NodeXY>
							<NodeXY>
							<delta>
								<node-XY2>
								<x>20</x>
								<y>885</y>
								</node-XY2>
							</delta>
							</NodeXY>
							<NodeXY>
							<delta>
								<node-XY4>
								<x>-296</x>
								<y>2261</y>
								</node-XY4>
							</delta>
							</NodeXY>
							<NodeXY>
							<delta>
								<node-XY3>
								<x>-527</x>
								<y>1756</y>
								</node-XY3>
							</delta>
							<attributes>
								<dElevation>-10</dElevation>
							</attributes>
							</NodeXY>
							<NodeXY>
							<delta>
								<node-XY1>
								<x>-408</x>
								<y>-66</y>
								</node-XY1>
							</delta>
							</NodeXY>
							<NodeXY>
							<delta>
								<node-XY3>
								<x>527</x>
								<y>-1652</y>
								</node-XY3>
							</delta>
							</NodeXY>
							<NodeXY>
							<delta>
								<node-XY2>
								<x>26</x>
								<y>-826</y>
								</node-XY2>
							</delta>
							<attributes>
								<dElevation>10</dElevation>
							</attributes>
							</NodeXY>
							<NodeXY>
							<delta>
								<node-XY3>
								<x>329</x>
								<y>-1186</y>
								</node-XY3>
							</delta>
							</NodeXY>
						</nodes>
						</xy>
					</offset>
					</path>
				</description>
				</GeographicalPath>
			</regions>
			<doNotUse3>0</doNotUse3>
			<doNotUse4>0</doNotUse4>
			<content>
				<workZone>
				<SEQUENCE>
					<item>
					<itis>769</itis>
					</item>
				</SEQUENCE>
				</workZone>
			</content>
			</TravelerDataFrame>
		</dataFrames>
	</TravelerInformation>
	)";
	string expectedHex = "001f582010000000000087faa72080b28cceeb1e7506be6a21b980026fd25877a5a028007e5199dd63cea0d7cd443700b73c0010030a4543a43053ba8ddb18d552f8f6e013ec01a1be15078c6260d0c60282852926bc0010018080";	
	#elif SAEJ2735_SPEC == 2020
	// 2020 TIM message
	string timStr="<TravelerInformation><msgCnt>1</msgCnt><packetID>00000000000F9E1D8D</packetID><dataFrames><TravelerDataFrame><notUsed>0</notUsed><frameType><unknown/></frameType><msgId><roadSignID><position><lat>389549153</lat><long>-771488965</long><elevation>400</elevation></position><viewAngle>0000000000000000</viewAngle><mutcdCode><none/></mutcdCode></roadSignID></msgId><startYear>2023</startYear><startTime>394574</startTime><durationTime>32000</durationTime><priority>5</priority><notUsed1>0</notUsed1><regions><GeographicalPath><anchor><lat>389549153</lat><long>-771488965</long><elevation>400</elevation></anchor><laneWidth>366</laneWidth><directionality><forward/></directionality><closedPath><false/></closedPath><direction>0000000000000000</direction><description><path><offset><xy><nodes><NodeXY><delta><node-LatLon><lon>-771489394</lon><lat>389549194</lat></node-LatLon></delta><attributes><dElevation>-10</dElevation></attributes></NodeXY><NodeXY><delta><node-LatLon><lon>-771487215</lon><lat>389548996</lat></node-LatLon></delta><attributes><dElevation>10</dElevation></attributes></NodeXY><NodeXY><delta><node-LatLon><lon>-771485210</lon><lat>389548981</lat></node-LatLon></delta><attributes><dElevation>10</dElevation></attributes></NodeXY></nodes></xy></offset></path></description></GeographicalPath></regions><notUsed2>0</notUsed2><notUsed3>0</notUsed3><content><speedLimit><SEQUENCE><item><itis>27</itis></item></SEQUENCE><SEQUENCE><item><text>Curve Ahead</text></item></SEQUENCE><SEQUENCE><item><itis>2564</itis></item></SEQUENCE><SEQUENCE><item><text>25</text></item></SEQUENCE><SEQUENCE><item><itis>8720</itis></item></SEQUENCE></speedLimit></content></TravelerDataFrame></dataFrames></TravelerInformation>";
	string expectedHex = "001f6820100000000000f9e1d8d0803299b9eac27a9baa74232000000fcec0a9df4028007e53373d584f53754e846400b720000000b8f5374e3666e7ac5013ece3d4ddc1099b9e988050538f5378f9666e7a5a814140034000dea1f5e5db2a083a32e1c80a048b26a22100";
	#else
	// 2016 TIM message
	string timStr="<TravelerInformation><msgCnt>1</msgCnt><timeStamp>115549</timeStamp><packetID>000000000023667BAC</packetID><dataFrames><TravelerDataFrame><sspTimRights>0</sspTimRights><frameType><advisory/></frameType><msgId><roadSignID><position><lat>389549775</lat><long>-771491835</long><elevation>390</elevation></position><viewAngle>1111111111111111</viewAngle><mutcdCode><warning/></mutcdCode></roadSignID></msgId><startTime>115549</startTime><duratonTime>1</duratonTime><priority>7</priority><sspLocationRights>0</sspLocationRights><regions><GeographicalPath><anchor><lat>389549775</lat><long>-771491835</long><elevation>390</elevation></anchor><directionality><both/></directionality><closedPath><true/></closedPath><description><geometry><direction>1111111111111111</direction><circle><center><lat>389549775</lat><long>-771491835</long><elevation>390</elevation></center><radius>74</radius><units><meter/></units></circle></geometry></description></GeographicalPath></regions><sspMsgRights1>0</sspMsgRights1><sspMsgRights2>0</sspMsgRights2><content><advisory><SEQUENCE><item><itis>7186</itis></item></SEQUENCE><SEQUENCE><item><text>curve</text></item></SEQUENCE><SEQUENCE><item><itis>13569</itis></item></SEQUENCE></advisory></content><url>987654321</url></TravelerDataFrame></dataFrames></TravelerInformation>";
	string expectedHex = "001f526011c35d000000000023667bac0407299b9ef9e7a9b9408230dfffe4386ba00078005a53373df3cf5372810461b90ffff53373df3cf53728104618129800010704a04c7d7976ca3501872e1bb66ad19b2620";
	#endif
	std::stringstream ss;
	TimMessage timMsg;
	TimEncodedMessage timEnc;
	tmx::message_container_type container;
	ss<<timStr;
	container.load<XML>(ss);
	timMsg.set_contents(container.get_storage().get_tree());
	timEnc.encode_j2735_message(timMsg);
	
	EXPECT_EQ(expectedHex, timEnc.get_payload_str());			
}

TEST(J2735MessageTest, EncodeSDSM)
{
	// Allocate a C-style struct and manage it with shared_ptr and a custom deleter.
	auto message = j2735::j2735_create<SdsmTraits>();
	message->msgCnt = 10;
	char  my_bytes_id[4] = {(char)1, (char)12, (char)12, (char)10};
	bool failed = OCTET_STRING_fromBuf(&message->sourceID, my_bytes_id, sizeof(my_bytes_id));
	ASSERT_EQ(failed, 0);
	message->equipmentType = EquipmentType_unknown;
	

	auto year = AllocAsn<DYear_t>();
	*year= 2023;
	message->sDSMTimeStamp.year = year;

	message->refPos.lat = 38121212;
	message->refPos.Long = -77121212;

	message->refPosXYConf.orientation = 10;
	message->refPosXYConf.semiMajor = 12;
	message->refPosXYConf.semiMinor = 52;

	auto objectData = AllocAsn<DetectedObjectData_t>();
	objectData->detObjCommon.objType = ObjectType_unknown;
	objectData->detObjCommon.objTypeCfd = 1;
	objectData->detObjCommon.objectID = 1;
	objectData->detObjCommon.measurementTime = 1;
	objectData->detObjCommon.timeConfidence = 1;
	objectData->detObjCommon.pos.offsetX = 1;
	objectData->detObjCommon.pos.offsetY = 1;
	objectData->detObjCommon.posConfidence.elevation = 1;
	objectData->detObjCommon.posConfidence.pos = 1;
	objectData->detObjCommon.speed = 1;
	objectData->detObjCommon.speedConfidence = 1;
	objectData->detObjCommon.heading = 1;
	objectData->detObjCommon.headingConf = 1;
	ASN_SEQUENCE_ADD(&message->objects.list.array, objectData);
	asn_fprint(stdout, &asn_DEF_SensorDataSharingMessage, message.get());

	//Encode SDSM 
	tmx::messages::SdsmEncodedMessage SdsmEncodeMessage;
	tmx::messages::SdsmMessage _sdsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_sdsmMessage.get_j2735_data());
	SdsmEncodeMessage.set_data(TmxJ2735EncodedMessage<SdsmMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));

	std::string expectedSDSMEncHex = "0029250a010c0c0a101f9c37ea97fc66b10b430c34000a00000020002bba0a000200004400240009";
	EXPECT_EQ(expectedSDSMEncHex, SdsmEncodeMessage.get_payload_str());	




	}

	TEST(J2735MessageTest, DecodeSDSM) {
		J2735MessageFactory factory;
		std::string hexString = "0029250a010c0c0a101f9c37ea97fc66b10b430c34000a00000020002bba0a000200004400240009";
		tmx::byte_stream bytes = tmx::byte_stream_decode(hexString);
		std::shared_ptr<MessageFrameEncodedMessage> msg = std::shared_ptr<MessageFrameEncodedMessage>(static_cast<MessageFrameEncodedMessage*>(factory.NewMessage(bytes)));
		auto decoded_msg = msg->decode_j2735_message();
		std::string expected_json_message = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<MessageFrame><messageId>41</messageId><value><SensorDataSharingMessage><msgCnt>10</msgCnt><sourceID>010C0C0A</sourceID><equipmentType><unknown/></equipmentType><sDSMTimeStamp><year>2023</year></sDSMTimeStamp><refPos><lat>38121212</lat><long>-77121212</long></refPos><refPosXYConf><semiMajor>12</semiMajor><semiMinor>52</semiMinor><orientation>10</orientation></refPosXYConf><objects><DetectedObjectData><detObjCommon><objType><unknown/></objType><objTypeCfd>1</objTypeCfd><objectID>1</objectID><measurementTime>1</measurementTime><timeConfidence><time-100-000/></timeConfidence><pos><offsetX>1</offsetX><offsetY>1</offsetY></pos><posConfidence><pos><a500m/></pos><elevation><elev-500-00/></elevation></posConfidence><speed>1</speed><speedConfidence><prec100ms/></speedConfidence><heading>1</heading><headingConf><prec10deg/></headingConf></detObjCommon></DetectedObjectData></objects></SensorDataSharingMessage></value></MessageFrame>";
		EXPECT_EQ(decoded_msg.to_string(),expected_json_message);
	}
}
