//============================================================================
// Name        : J2735MessageTest.cpp
// Description : Unit tests for the J2735 Message library.
//============================================================================

#include <boost/any.hpp>
#include <gtest/gtest.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <tmx/j2735_messages/testMessage03.hpp>
#include <tmx/messages/message_document.hpp>
#include <vector>
#include <iostream>
#include <chrono>
#include <sstream>
#include <cassert>

using namespace std;
using namespace battelle::attributes;
using namespace tmx;
using namespace tmx::messages;

namespace unit_test {

class msg_type {
public:
	msg_type() {}
	virtual ~msg_type() {}

	virtual string type_name() = 0;
	virtual string message_tag() = 0;
	virtual bool set_message(routeable_message *msg) = 0;
	virtual bool set_message(string) = 0;
	virtual int get_default_msgId() = 0;
	virtual int get_msgId() = 0;
	virtual byte_stream get_bytes() = 0;
	virtual string get_payload() = 0;
};

template <class MsgClass>
class msg_type_impl: public msg_type {
public:
	typedef MsgClass encoded_type;
	typedef typename MsgClass::message_type message_type;

	msg_type_impl(): msg_type() {}
	~msg_type_impl() {}

	string type_name()
	{
		return type_id_name<MsgClass>();
	}

	string message_tag()
	{
		return MsgClass::get_messageTag();
	}

	bool set_message(routeable_message *msg)
	{
		try
		{
			encMsg = dynamic_cast<TmxJ2735EncodedMessage<MsgClass> *>(msg);
		}
		catch (exception &ex)
		{
			cerr << ex.what() << endl;
		}

		return encMsg;
	}

	bool set_message(string msg)
	{
		static TmxJ2735EncodedMessage<MsgClass> enc;
		static string encoding = enc.get_encoding();
		enc.clear();
		enc.set_encoding(encoding);

		try
		{
			MsgClass dec;
			dec.set_contents(msg);
			enc.initialize(dec);
			encMsg = &enc;
		}
		catch (exception &ex)
		{
			cerr << ex.what() << endl;
			encMsg = NULL;
		}

		return encMsg;
	}

	int get_default_msgId()
	{
		return 0;
		//return MsgClass::get_default_msgId();
	}

	int get_msgId()
	{
		return encMsg->get_msgId();
	}

	byte_stream get_bytes()
	{
		return encMsg->get_data();
	}

	string get_payload()
	{
		return encMsg->template get_payload<MsgClass>().to_string();
	}
private:
	TmxJ2735EncodedMessage<MsgClass> *encMsg = NULL;
};

class J2735MessageTest: public testing::Test {

protected:
	J2735MessageTest() {
		if (enumNames[api::J2735].length() <= 0) {
			// Initialize the known names
			enumNames[api::J2735] = api::MSGSUBTYPE_J2735_STRING;

			enumNames[api::basicSafetyMessage_D] = api::MSGSUBTYPE_BASICSAFETYMESSAGE_D_STRING;
			msgTypes[api::basicSafetyMessage_D] = new msg_type_impl<BsmMessage>();
			testBytes[api::basicSafetyMessage_D] = "302b8001028126003ade68b1000017dee47ece7000b00000ffffffff032451407f00000000b000700000000000";
			//enumNames[api::basicSafetyMessage] = api::MSGSUBTYPE_BASICSAFETYMESSAGE_STRING;
			enumNames[api::basicSafetyMessageVerbose_D] = api::MSGSUBTYPE_BASICSAFETYMESSAGEVERBOSE_D_STRING;
			msgTypes[api::basicSafetyMessageVerbose_D] = new msg_type_impl<BsmMessage>();
			enumNames[api::commonSafetyRequest_D] = api::MSGSUBTYPE_COMMONSAFETYREQUEST_D_STRING;
			msgTypes[api::commonSafetyRequest_D] = new msg_type_impl<CsrMessage>();
			//enumNames[api::commonSafetyRequest] = api::MSGSUBTYPE_COMMONSAFETYREQUEST_STRING;
			enumNames[api::emergencyVehicleAlert_D] = api::MSGSUBTYPE_EMERGENCYVEHICLEALERT_D_STRING;
			msgTypes[api::emergencyVehicleAlert_D] = new msg_type_impl<EvaMessage>();
			//enumNames[api::emergencyVehicleAlert] = api::MSGSUBTYPE_EMERGENCYVEHICLEALERT_STRING;
			enumNames[api::intersectionCollision_D] = api::MSGSUBTYPE_INTERSECTIONCOLLISION_D_STRING;
			msgTypes[api::intersectionCollision_D] = new msg_type_impl<IntersectionCollisionMessage>();
			//enumNames[api::intersectionCollision] = api::MSGSUBTYPE_INTERSECTIONCOLLISION_STRING;
			//enumNames[api::mapData_D] = api::MSGSUBTYPE_MAPDATA_D_STRING;
			enumNames[api::mapData] = api::MSGSUBTYPE_MAPDATA_STRING;
			msgTypes[api::mapData] = new msg_type_impl<MapDataMessage>();
			testBytes[api::mapData] = "30819c80011183011284819328048018081098011360f4970e6e86ecc0bc028600a000000000c800468cca454266798e048305100860230018400002001b3db8b730056c35887a90880ad80483801010602a000000000c80158fd46004fd37182a1c4809fa0481051008603b001840000200110a52c056c354b8adb880ad80482001010401100000000001000dffff05da100c40000000000400b7fffbe888";
			enumNames[api::nmeaCorrections_D] = api::MSGSUBTYPE_NMEACORRECTIONS_D_STRING;
			msgTypes[api::nmeaCorrections_D] = new msg_type_impl<NmeaMessage>();
			//enumNames[api::nmeaCorrections] = api::MSGSUBTYPE_NMEACORRECTIONS_STRING;
			enumNames[api::personalSafetyMessage] = api::MSGSUBTYPE_PERSONALSAFETYMESSAGE_STRING;
			msgTypes[api::personalSafetyMessage] = new msg_type_impl<PsmMessage>();
			testBytes[api::personalSafetyMessage] = "30218001118301208419000008000012448217435a4e9006b49d1ff323034310000100";
			enumNames[api::probeDataManagement_D] = api::MSGSUBTYPE_PROBEDATAMANAGEMENT_D_STRING;
			msgTypes[api::probeDataManagement_D] = new msg_type_impl<PdmMessage>();
			//enumNames[api::probeDataManagement] = api::MSGSUBTYPE_PROBEDATAMANAGEMENT_STRING;
			enumNames[api::probeVehicleData_D] = api::MSGSUBTYPE_PROBEVEHICLEDATA_D_STRING;
			msgTypes[api::probeVehicleData_D] = new msg_type_impl<PvdMessage>();
			testBytes[api::probeVehicleData_D] = "302680010a810103a3098101b482012b84016a840106850101a60d300ba00981012d8201b484016a";
			//enumNames[api::probeVehicleData] = api::MSGSUBTYPE_PROBEVEHICLEDATA_STRING;
			enumNames[api::roadSideAlert_D] = api::MSGSUBTYPE_ROADSIDEALERT_D_STRING;
			msgTypes[api::roadSideAlert_D] = new msg_type_impl<RsaMessage>();
			testBytes[api::roadSideAlert_D] = "302780010b81010082021311a30084008500860100a70ea0008104ce7049b4820417dee95c88008900";
			//enumNames[api::roadSideAlert] = api::MSGSUBTYPE_ROADSIDEALERT_STRING;
			enumNames[api::rtcmCorrections_D] = api::MSGSUBTYPE_RTCMCORRECTIONS_D_STRING;
			msgTypes[api::rtcmCorrections_D] = new msg_type_impl<RtcmMessage>();
			//enumNames[api::rtcmCorrections] = api::MSGSUBTYPE_RTCMCORRECTIONS_STRING;
			//enumNames[api::signalPhaseAndTimingMessage_D] = api::MSGSUBTYPE_SIGNALPHASEANDTIMINGMESSAGE_D_STRING;
			enumNames[api::signalPhaseAndTimingMessage] = api::MSGSUBTYPE_SIGNALPHASEANDTIMINGMESSAGE_STRING;
			msgTypes[api::signalPhaseAndTimingMessage] = new msg_type_impl<SpatMessage>();
			testBytes[api::signalPhaseAndTimingMessage] = "30818080011183011384780130201b6bd3420c3bb220a9a79e4c3b32a093bba65e5cf2e3e9a77ee03c48100000b00204343a663a66001021a1d331d33001410c0e49800a086874cc74cc0040454396c396c002022a1cb61cb6001c10c0e49800e08a872d872d800604343a663a66003021a1d331d33000410c0e498002086874cc74ccfd002020602a000000001d0054f3857404e2adefc542027154f886dc04e2827f0008143009000000000748017022f7fff14fd0008127e001008300d000000000e802a03a7ce02715879f419004e2ac4590a00271413f40040618108000000003a40077fffbf5f8a7e0004213ec0081008d4c0010000004944c8804bb2b479f3809760400b50234300040000015a011b804f325d8090027981004d008ccc0010000004fca0f804e3acf990320271c1006cc08d8c00100000050f92d4014529851de80a29040236047d100000000001000e0706fffe11f040000000000400f7fffbf69847b100000000001002dfa86fffe11f84000000000040077fffc09a8";
			enumNames[api::signalRequestMessage_D] = api::MSGSUBTYPE_SIGNALREQUESTMESSAGE_D_STRING;
			msgTypes[api::signalRequestMessage_D] = new msg_type_impl<SrmMessage>();
			//enumNames[api::signalRequestMessage] = api::MSGSUBTYPE_SIGNALREQUESTMESSAGE_STRING;
			enumNames[api::signalStatusMessage_D] = api::MSGSUBTYPE_SIGNALSTATUSMESSAGE_D_STRING;
			msgTypes[api::signalStatusMessage_D] = new msg_type_impl<SsmMessage>();
			//enumNames[api::signalStatusMessage] = api::MSGSUBTYPE_SIGNALSTATUSMESSAGE_STRING;
			enumNames[api::travelerInformation_D] = api::MSGSUBTYPE_TRAVELERINFORMATION_D_STRING;
			msgTypes[api::travelerInformation_D] = new msg_type_impl<TimMessage>();
			testBytes[api::travelerInformation_D] = "3082011f8001108104000037bf830101a482010d30820109800101a102800083022d00840300fde8850105a981cc30318000a22da02ba010800417bab1ea8104c18d80cd820200008102014f820100a31004060000000000000406fff9010c000030318000a22da02ba010800417baa6fa8104c18d80cd820200008102014f820100a3100406000000000000040600000137000030318000a22da02ba010800417ba98cc8104c18d80d6820200008102014f820100a31004060000000000000406ffff0194000030318000a22da02ba010800417ba86f28104c18d80b8820200008102014f820100a31004060000000000000406000301fc0000aa25a0233005a00380011b3008a0068004016ee8003006a00480020a043008a0068004016ee9008500";
			//enumNames[api::travelerInformation] = api::MSGSUBTYPE_TRAVELERINFORMATION_STRING;
			enumNames[api::uperFrame_D] = api::MSGSUBTYPE_UPERFRAME_D_STRING;
		//	msgTypes[api::uperFrame_D] = new msg_type_impl<UperFrameMessage>();
			#if SAEJ2735_SPEC < 2020
			enumNames[api::personalMobilityMessage] = api::MSGSUBTYPE_PERSONALMOBILITYMESSAGE_STRING;
		//	msgTypes[api::personalMobilityMessage] = new msg_type_impl<PmmMessage>();
			testBytes[api::personalMobilityMessage] = "303a800111830200f58431482362c99e568d5b375b95c39c4b58b2c8cd6e168d5b2d68c9b366ad5a3460c1830000d693a401ad2747fc7e09b3720034";
			#endif
		}
	}

	virtual ~J2735MessageTest() {
		for (unsigned int i = 0; i < msgTypes.size(); i++)
			if (msgTypes[i])
			{
				delete msgTypes[i];
				msgTypes[i] = NULL;
			}
	}

	J2735MessageFactory factory;
	static vector<string> enumNames;
	static vector<msg_type *> msgTypes;
	static vector<string> testBytes;
};

vector<string> J2735MessageTest::enumNames(api::J2735_end);
vector<msg_type *> J2735MessageTest::msgTypes(api::J2735_end);
vector<string> J2735MessageTest::testBytes(api::J2735_end);

TEST_F(J2735MessageTest, EncodeMobilityOperation)
{	
	TestMessage03_t* message = (TestMessage03_t*) malloc( sizeof(TestMessage03_t) );

	/**
	 * Populate MobilityHeader 
	 */
	
	char* my_str = (char *) "sender_id";
	uint8_t * my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->header.hostStaticId.buf = my_bytes;
	message->header.hostStaticId.size = strlen(my_str);
	message->header.targetStaticId.buf = my_bytes;
	message->header.targetStaticId.size = strlen(my_str);

	my_str = (char *) "bsm_idXX";
	my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->header.hostBSMId.buf = my_bytes;
	message->header.hostBSMId.size = strlen(my_str);

	my_str = (char *) "00000000-0000-0000-0000-000000000000";
	my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->header.planId.buf = my_bytes;
	message->header.planId.size = strlen(my_str);

	unsigned long timestamp_ll = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now().time_since_epoch()).count();		
	std::string timestamp_str = std::to_string(timestamp_ll).c_str();
	char * my_str_1 = new char[strlen(timestamp_str.c_str())];
	uint8_t * my_bytes_1 = new uint8_t[strlen(timestamp_str.c_str())];
	strcpy(my_str_1, timestamp_str.c_str());
	for(int i = 0; i< strlen(my_str_1); i++)
	{
		my_bytes_1[i] =  (uint8_t)my_str_1[i];
	}
	message->header.timestamp.buf = my_bytes_1;
	message->header.timestamp.size = strlen(my_str_1);

	/**
	 * Populate MobilityOperation Body 
	 */
	my_str = (char *) "traffic_control_id: traffic_control_id, acknowledgement: true, reason: optional reason text";
	my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->body.operationParams.buf = my_bytes;
	message->body.operationParams.size = strlen(my_str);

	my_str = (char *) "carma3/Geofence_Acknowledgement";
	my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->body.strategy.buf = my_bytes;
	message->body.strategy.size = strlen(my_str);

	tmx::messages::tsm3EncodedMessage tsm3EncodeMessage;
	tmx::messages::tsm3Message*  _tsm3Message = new tmx::messages::tsm3Message(message);
	tmx::messages::MessageFrameMessage frame_msg(_tsm3Message->get_j2735_data());
	tsm3EncodeMessage.set_data(TmxJ2735EncodedMessage<TestMessage03>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
		
	free(message);
	delete my_bytes_1;
	delete my_str_1;
	free(frame_msg.get_j2735_data().get());	
	ASSERT_EQ(243,  tsm3EncodeMessage.get_msgId());
}


TEST_F(J2735MessageTest, EncodeMobilityRequest)
{	
	TestMessage00_t* message = (TestMessage00_t*) calloc(1, sizeof(TestMessage00_t) );

	/**
	 * Populate MobilityHeader 
	 */
	
	char* my_str = (char *) "sender_id";
	uint8_t* my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->header.hostStaticId.buf = my_bytes;
	message->header.hostStaticId.size = strlen(my_str);
	message->header.targetStaticId.buf = my_bytes;
	message->header.targetStaticId.size = strlen(my_str);

	my_str = (char *) "bsm_idXX";
	my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->header.hostBSMId.buf = my_bytes;
	message->header.hostBSMId.size = strlen(my_str);

	my_str = (char *) "00000000-0000-0000-0000-000000000000";
	my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->header.planId.buf = my_bytes;
	message->header.planId.size = strlen(my_str);

	unsigned long timestamp_ll = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now().time_since_epoch()).count();		
	std::string timestamp_str = std::to_string(timestamp_ll).c_str();
	char * my_str_1 = new char[strlen(timestamp_str.c_str())];
	uint8_t * my_bytes_1 = new uint8_t[strlen(timestamp_str.c_str())];
	strcpy(my_str_1, timestamp_str.c_str());
	for(int i = 0; i< strlen(my_str_1); i++)
	{
		my_bytes_1[i] =  (uint8_t)my_str_1[i];
	}
	message->header.timestamp.buf = my_bytes_1;
	message->header.timestamp.size = strlen(my_str_1);

	/**
	 * Populate MobilityRequest Body 
	 */
	my_str = (char *) "traffic_control_id: traffic_control_id, acknowledgement: true, reason: optional reason text";
	my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->body.strategyParams.buf = my_bytes;
	message->body.strategyParams.size = strlen(my_str);

	my_str = (char *) "carma3/Geofence_Acknowledgement";
	my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->body.strategy.buf = my_bytes;
	message->body.strategy.size = strlen(my_str);

	message->body.urgency = 1;
	message->body.planType = 0;
	message->body.location.ecefX = 1;
	message->body.location.ecefY = 1;
	message->body.location.ecefZ = 1;
	message->body.location.timestamp.buf = my_bytes_1;
	message->body.location.timestamp.size = strlen(my_str_1);
	
	message->body.expiration.buf = my_bytes_1;
	message->body.expiration.size = strlen(my_str_1);
	
		
	MobilityECEFOffset_t* offset = (MobilityECEFOffset_t*)calloc(1, sizeof(MobilityECEFOffset_t) );
	offset->offsetX = 1;
	offset->offsetY = 1;
	offset->offsetZ = 1;
	ASN_SEQUENCE_ADD(&message->body.trajectory.list.array, offset);
	ASN_SEQUENCE_ADD(&message->body.trajectory.list.array, offset);

	message->body.trajectoryStart.ecefX = 1;
	message->body.trajectoryStart.ecefY = 1;
	message->body.trajectoryStart.ecefZ = 1;
	message->body.trajectoryStart.timestamp.buf = my_bytes_1;
	message->body.trajectoryStart.timestamp.size = strlen(my_str_1);
		
	tmx::messages::tsm0EncodedMessage tsm0EncodeMessage;
	tmx::messages::tsm0Message*  _tsm0Message = new tmx::messages::tsm0Message(message);
	tmx::messages::MessageFrameMessage frame_msg(_tsm0Message->get_j2735_data());
	tsm0EncodeMessage.set_data(TmxJ2735EncodedMessage<TestMessage00>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
		
	free(message);
	delete my_bytes_1;
	delete my_str_1;
	free(offset);
	free(frame_msg.get_j2735_data().get());
	ASSERT_EQ(240,  tsm0EncodeMessage.get_msgId());
}


TEST_F(J2735MessageTest, EncodeMobilityResponse)
{	
	TestMessage01_t* message = (TestMessage01_t*) malloc( sizeof(TestMessage01_t) );

	/**
	 * Populate MobilityHeader 
	 */
	
	char* my_str = (char *) "sender_id";
	uint8_t* my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->header.hostStaticId.buf = my_bytes;
	message->header.hostStaticId.size = strlen(my_str);
	message->header.targetStaticId.buf = my_bytes;
	message->header.targetStaticId.size = strlen(my_str);

	my_str = (char *) "bsm_idXX";
	my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->header.hostBSMId.buf = my_bytes;
	message->header.hostBSMId.size = strlen(my_str);

	my_str = (char *) "00000000-0000-0000-0000-000000000000";
	my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->header.planId.buf = my_bytes;
	message->header.planId.size = strlen(my_str);

	unsigned long timestamp_ll = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::system_clock::now().time_since_epoch()).count();		
	std::string timestamp_str = std::to_string(timestamp_ll).c_str();
	char * my_str_1 = new char[strlen(timestamp_str.c_str())];
	uint8_t * my_bytes_1 = new uint8_t[strlen(timestamp_str.c_str())];
	strcpy(my_str_1, timestamp_str.c_str());
	for(int i = 0; i< strlen(my_str_1); i++)
	{
		my_bytes_1[i] =  (uint8_t)my_str_1[i];
	}
	message->header.timestamp.buf = my_bytes_1;
	message->header.timestamp.size = strlen(my_str_1);

	/**
	 * Populate MobilityResponse Body 
	 */
	message->body.isAccepted = 1;
	message->body.urgency = 1;

	tmx::messages::tsm1EncodedMessage tsm1EncodeMessage;
	tmx::messages::tsm1Message*  _tsm1Message = new tmx::messages::tsm1Message(message);
	tmx::messages::MessageFrameMessage frame_msg(_tsm1Message->get_j2735_data());
	tsm1EncodeMessage.set_data(TmxJ2735EncodedMessage<TestMessage01>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
		
	free(message);
	delete my_bytes_1;
	delete my_str_1;
	free(frame_msg.get_j2735_data().get());
	ASSERT_EQ(241,  tsm1EncodeMessage.get_msgId());
}


TEST_F(J2735MessageTest, EncodeBasicSafetyMessage)
{	
	BasicSafetyMessage_t* message = (BasicSafetyMessage_t*) calloc(1, sizeof(BasicSafetyMessage_t) );

	/**
	 * Populate BSMcoreData 
	 */
	
	char* my_str = (char *) "sender_id";
	uint8_t* my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->coreData.msgCnt = 1;
	uint8_t  my_bytes_id[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
	message->coreData.id.buf = my_bytes_id;
	message->coreData.id.size = sizeof(my_bytes_id);
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
	uint8_t  my_bytes_brakes[1] = {8};
	message->coreData.brakes.wheelBrakes.buf = my_bytes_brakes; // allow 0,1,2,3,4
	message->coreData.brakes.wheelBrakes.size = sizeof(my_bytes_brakes); // allow 0,1,2,3,4	
	message->coreData.brakes.wheelBrakes.bits_unused = 3; // allow 0,1,2,3,4	

	//vehicle size
	message->coreData.size.length = 500;
	message->coreData.size.width = 300;

	tmx::messages::BsmEncodedMessage bsmEncodeMessage;
	tmx::messages::BsmMessage*  _bsmMessage = new tmx::messages::BsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_bsmMessage->get_j2735_data());
	bsmEncodeMessage.set_data(TmxJ2735EncodedMessage<BasicSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
		
	free(message);
	free(frame_msg.get_j2735_data().get());
	ASSERT_EQ(20,  bsmEncodeMessage.get_msgId());
	//Decode the encoded BSM
	auto bsm_ptr = bsmEncodeMessage.decode_j2735_message().get_j2735_data();
}



TEST_F(J2735MessageTest, EncodeBasicSafetyMessage_PartII)
{	
	BasicSafetyMessage_t* message = (BasicSafetyMessage_t*) calloc(1, sizeof(BasicSafetyMessage_t) );
	/**
	 * Populate BSMcoreData 
	*/	
	char* my_str = (char *) "sender_id";
	uint8_t* my_bytes = reinterpret_cast<uint8_t *>(my_str);
	message->coreData.msgCnt = 1;
	uint8_t  my_bytes_id[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
	message->coreData.id.buf = my_bytes_id;
	message->coreData.id.size = sizeof(my_bytes_id);
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
	uint8_t  my_bytes_brakes[1] = {8};
	message->coreData.brakes.wheelBrakes.buf = my_bytes_brakes; // allow 0,1,2,3,4
	message->coreData.brakes.wheelBrakes.size = sizeof(my_bytes_brakes); // allow 0,1,2,3,4	
	message->coreData.brakes.wheelBrakes.bits_unused = 3; // allow 0,1,2,3,4	

	//vehicle size
	message->coreData.size.length = 500;
	message->coreData.size.width = 300;

	//BSM BSMpartIIExtension
	auto bsmPartII = (BasicSafetyMessage::BasicSafetyMessage__partII*) calloc(1, sizeof(BasicSafetyMessage::BasicSafetyMessage__partII));
	auto partIICnt = (BSMpartIIExtension_t*) calloc(1, sizeof(BSMpartIIExtension_t));
	partIICnt->partII_Id = 1;
	partIICnt->partII_Value.present = BSMpartIIExtension__partII_Value_PR_SpecialVehicleExtensions;
	auto specialVEx = (SpecialVehicleExtensions_t*) calloc(1, sizeof(SpecialVehicleExtensions_t));
	auto emergencyDetails = (EmergencyDetails_t*) calloc(1, sizeof(EmergencyDetails_t));
	emergencyDetails->lightsUse = LightbarInUse_inUse;
	auto resp_type = (ResponseType_t*) calloc(1, sizeof(ResponseType_t));
	*resp_type = ResponseType_emergency;
	emergencyDetails->responseType = resp_type;
	emergencyDetails->sirenUse = SirenInUse_inUse;	
	specialVEx->vehicleAlerts = emergencyDetails;
	partIICnt->partII_Value.choice.SpecialVehicleExtensions = *specialVEx;
    asn_sequence_add(&bsmPartII->list.array, partIICnt);
	message->partII = bsmPartII;

	// BSM regional extension
    auto regional = (BasicSafetyMessage::BasicSafetyMessage__regional*) calloc(1, sizeof(BasicSafetyMessage::BasicSafetyMessage__regional));
    auto reg_bsm = (Reg_BasicSafetyMessage_t*) calloc(1, sizeof(Reg_BasicSafetyMessage_t));
    reg_bsm->regionId = 128;
    reg_bsm->regExtValue.present = Reg_BasicSafetyMessage__regExtValue_PR_BasicSafetyMessage_addGrpCarma;

    auto carma_bsm_data = (BasicSafetyMessage_addGrpCarma_t*) calloc(1, sizeof(BasicSafetyMessage_addGrpCarma_t));
    auto carma_bsm_destination_points = (BasicSafetyMessage_addGrpCarma::BasicSafetyMessage_addGrpCarma__routeDestinationPoints*) calloc(1, sizeof(BasicSafetyMessage_addGrpCarma::BasicSafetyMessage_addGrpCarma__routeDestinationPoints));
    auto point = (Position3D_t*) calloc(1, sizeof(Position3D_t));
	auto dummy_lat = 12;
	auto dummy_long = 1312;
    point->lat = dummy_lat;
    point->Long = dummy_long;
    asn_sequence_add(&carma_bsm_destination_points->list.array, point);
    auto point2 = (Position3D_t*) calloc(1, sizeof(Position3D_t));
    point2->lat = dummy_lat + 1000;
    point2->Long = dummy_long + 1000;
    asn_sequence_add(&carma_bsm_destination_points->list.array, point2);
    carma_bsm_data->routeDestinationPoints = carma_bsm_destination_points;
    reg_bsm->regExtValue.choice.BasicSafetyMessage_addGrpCarma = *carma_bsm_data;

    asn_sequence_add(&regional->list.array, reg_bsm);
    message->regional = regional;

	xer_fprint(stdout, &asn_DEF_BasicSafetyMessage, message);

	//Encode BSM
	tmx::messages::BsmEncodedMessage bsmEncodeMessage;
	tmx::messages::BsmMessage*  _bsmMessage = new tmx::messages::BsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_bsmMessage->get_j2735_data());
	bsmEncodeMessage.set_data(TmxJ2735EncodedMessage<BasicSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
	free(message);
	free(frame_msg.get_j2735_data().get());
	ASSERT_EQ(20,  bsmEncodeMessage.get_msgId());	
	std::string expectedBSMEncHex = "00143d604043030280ffdbfba868b3584ec40824646400320032000c888fc834e37fff0aaa960fa0040d082408801148d693a431ad275c7c6b49d9e8d693b60e";
	ASSERT_EQ(expectedBSMEncHex, bsmEncodeMessage.get_payload_str());

	//Decode the encoded BSM
	auto decoded_bsm_ptr = bsmEncodeMessage.decode_j2735_message().get_j2735_data();
	ASSERT_EQ(LightbarInUse_inUse,  decoded_bsm_ptr->partII->list.array[0]->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->lightsUse);
	ASSERT_EQ(SirenInUse_inUse,  decoded_bsm_ptr->partII->list.array[0]->partII_Value.choice.SpecialVehicleExtensions.vehicleAlerts->sirenUse);
	auto decoded_regional = (BasicSafetyMessage::BasicSafetyMessage__regional *)calloc(1, sizeof(BasicSafetyMessage::BasicSafetyMessage__regional));
	auto decoded_reg_bsm = (Reg_BasicSafetyMessage_t *)calloc(1, sizeof(Reg_BasicSafetyMessage_t));
	auto decode_carma_bsm_data = (BasicSafetyMessage_addGrpCarma_t *)calloc(1, sizeof(BasicSafetyMessage_addGrpCarma_t));
	decoded_regional = decoded_bsm_ptr->regional;
	decoded_reg_bsm = decoded_regional->list.array[0];
	ASSERT_EQ(dummy_lat,  decoded_bsm_ptr->regional->list.array[0]->regExtValue.choice.BasicSafetyMessage_addGrpCarma.routeDestinationPoints->list.array[0]->lat);
	ASSERT_EQ(dummy_long,  decoded_bsm_ptr->regional->list.array[0]->regExtValue.choice.BasicSafetyMessage_addGrpCarma.routeDestinationPoints->list.array[0]->Long);
	ASSERT_EQ(dummy_lat + 1000,  decoded_bsm_ptr->regional->list.array[0]->regExtValue.choice.BasicSafetyMessage_addGrpCarma.routeDestinationPoints->list.array[1]->lat);
	ASSERT_EQ(dummy_long + 1000,  decoded_bsm_ptr->regional->list.array[0]->regExtValue.choice.BasicSafetyMessage_addGrpCarma.routeDestinationPoints->list.array[1]->Long);
}


TEST_F(J2735MessageTest, EncodePersonalSafetyMessage){
	string psm="<PersonalSafetyMessage><basicType><aPEDESTRIAN/></basicType><secMark>109</secMark><msgCnt>0</msgCnt><id>115eadf0</id><position><lat>389549376</lat><long>-771491840</long></position><accuracy><semiMajor>255</semiMajor><semiMinor>255</semiMinor><orientation>65535</orientation></accuracy><speed>0</speed><heading>16010</heading><pathHistory><crumbData><PathHistoryPoint><latOffset>0</latOffset><lonOffset>0</lonOffset><elevationOffset>0</elevationOffset><timeOffset>1</timeOffset></PathHistoryPoint></crumbData></pathHistory></PersonalSafetyMessage>";
	std::stringstream ss;
	PsmMessage psmmessage;
	PsmEncodedMessage psmENC;
	tmx::message_container_type container;
	ss<<psm;
	container.load<XML>(ss);
	psmmessage.set_contents(container.get_storage().get_tree());
	psmENC.encode_j2735_message(psmmessage);
	std::cout << psmENC.get_payload_str() << std::endl;
	ASSERT_EQ(32,  psmENC.get_msgId());
}

TEST_F(J2735MessageTest, EncodeRoadSafetyMessage)
{
	// Encode RSM XML
	string rsm="<RoadSafetyMessage> <commonContainer> <eventInfo> <eventID> <operatorID> <fullRdAuthID>0.1.3.6.1</fullRdAuthID> </operatorID> <uniqueID>01 0C 0C 0A</uniqueID> </eventID> <eventUpdate>12</eventUpdate> <eventCancellation><false/></eventCancellation> <startDateTime> <year>2024</year> <month>3</month> <day>19</day> <hour>15</hour> <minute>30</minute> <second>45</second> </startDateTime> <eventRecurrence> <EventRecurrence> <monday><true/></monday> <tuesday><true/></tuesday> <wednesday><true/></wednesday> <thursday><true/></thursday> <friday><true/></friday> <saturday><true/></saturday> <sunday><true/></sunday> </EventRecurrence> </eventRecurrence> <causeCode>7</causeCode> <subCauseCode>1793</subCauseCode> <affectedVehicles><all-vehicles/> </affectedVehicles> </eventInfo> <regionInfo> <RegionInfo> <referencePoint> <lat>389549610</lat> <long>-771493030</long> <elevation>390</elevation> </referencePoint> </RegionInfo> </regionInfo> </commonContainer> <content> <dynamicInfoContainer> <priority><critical/></priority> <dmsSignString> <ShortString>Wrong Way Driver</ShortString> </dmsSignString> <applicableRegion> <referencePoint> <lat>389549610</lat> <long>-771493030</long> <elevation>390</elevation> </referencePoint> </applicableRegion> </dynamicInfoContainer> </content> </RoadSafetyMessage>";
	std::stringstream ss;
	RsmMessage rsmmessage;
	RsmEncodedMessage rsmENC;
	tmx::message_container_type container;
	ss<<rsm;
	container.load<XML>(ss);
	rsmmessage.set_contents(container.get_storage().get_tree());
	rsmENC.encode_j2735_message(rsmmessage);
	std::cout << rsmENC.get_payload_str() << std::endl;
	ASSERT_EQ(33,  rsmENC.get_msgId());


	/**
	 * Populate RSM 
	*/	
	auto message = (RoadSafetyMessage_t*) calloc(1, sizeof(RoadSafetyMessage_t));
	auto commonContainer = (CommonContainer_t*) calloc(1, sizeof(CommonContainer_t));
	auto eventInfo = (EventInfo_t*) calloc(1, sizeof(EventInfo_t));

	// Event ID info
	auto eventID = (EventIdentifier_t*) calloc(1, sizeof(EventIdentifier_t));
	eventID->operatorID.present = RoadAuthorityID_PR_fullRdAuthID;
	uint8_t my_bytes_oid[4] = {(uint8_t)1, (uint8_t)3, (uint8_t)6, (uint8_t)1};
	uint8_t  my_bytes_uid[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
	eventID->operatorID.choice.fullRdAuthID.buf = my_bytes_oid;
	eventID->operatorID.choice.fullRdAuthID.size = sizeof(my_bytes_oid);
	eventID->uniqueID.buf = my_bytes_uid;
	eventID->uniqueID.size = sizeof(my_bytes_uid);
	eventInfo->eventID = *eventID;

	// Event update version
	eventInfo->eventUpdate = 12;

	// Is event cancelled
	eventInfo->eventCancellation = 0;

	// Start Date time
	auto rsmStartDateTime = (DDateTime_t*) calloc(1, sizeof(DDateTime_t));
	auto year = (DYear_t*) calloc(1, sizeof(DYear_t));
	auto month = (DMonth_t*) calloc(1, sizeof(DMonth_t));
	auto day = (DDay_t*) calloc(1, sizeof(DDay_t));
	auto hour = (DHour_t*) calloc(1, sizeof(DHour_t));
	auto minute = (DMinute_t*) calloc(1, sizeof(DMinute_t));
	auto second = (DSecond_t*) calloc(1, sizeof(DSecond_t));
	*year = 2024;
	*month = 5;
	*day = 13;
	*hour = 12;
	*minute = 0;
	*second = 0;
	rsmStartDateTime->year = year;
	rsmStartDateTime->month = month;
	rsmStartDateTime->day = day;
	rsmStartDateTime->hour = hour;
	rsmStartDateTime->minute = minute;
	rsmStartDateTime->second = second;
	eventInfo->startDateTime = *rsmStartDateTime;

	// // End Date time
	// auto rsmEndDateTime = (DDateTime_t*) calloc(1, sizeof(DDateTime_t));
	// auto endYear = (DYear_t*) calloc(1, sizeof(DYear_t));
	// auto endMonth = (DMonth_t*) calloc(1, sizeof(DMonth_t));
	// auto endDay = (DDay_t*) calloc(1, sizeof(DDay_t));
	// auto endHour = (DHour_t*) calloc(1, sizeof(DHour_t));
	// auto endMinute = (DMinute_t*) calloc(1, sizeof(DMinute_t));
	// auto endSecond = (DSecond_t*) calloc(1, sizeof(DSecond_t));
	// *endYear = 2024;
	// *endMonth = 5;
	// *endDay = 17;
	// *endHour = 12;
	// *endMinute = 0;
	// *endSecond = 0;
	// rsmEndDateTime->year = endYear;
	// rsmEndDateTime->month = endMonth;
	// rsmEndDateTime->day = endDay;
	// rsmEndDateTime->hour = endHour;
	// rsmEndDateTime->minute = endMinute;
	// rsmEndDateTime->second = endSecond;
	// eventInfo->endDateTime = rsmEndDateTime;

	// // Event recurrence list
	// auto rsmEventRecurrence = (EventInfo::EventInfo__eventRecurrence*) calloc(1, sizeof(EventInfo::EventInfo__eventRecurrence));
	// auto eventRecCnt = (EventRecurrence_t*) calloc(1, sizeof(EventRecurrence_t));
	// eventRecCnt->monday = 1;
	// eventRecCnt->tuesday = 1;
	// eventRecCnt->wednesday = 1;
	// eventRecCnt->thursday = 1;
	// eventRecCnt->friday = 1;
	// eventRecCnt->saturday = 1;
	// eventRecCnt->sunday = 1;
	// asn_sequence_add(&rsmEventRecurrence->list.array, eventRecCnt);
	// eventInfo->eventRecurrence = rsmEventRecurrence;

	// ITIS cause codes
	eventInfo->causeCode = 2;
	// auto subCode = (ITIS_ITIScodes_t*) calloc(1, sizeof(ITIS_ITIScodes_t));
	// *subCode = 8026;
	// eventInfo->subCauseCode = subCode;

	// ITIS affected vehicle code list
	auto rsmAffectedVehicles = (EventInfo::EventInfo__affectedVehicles*) calloc(1, sizeof(EventInfo::EventInfo__affectedVehicles));
	auto affectedVehCnt = (ITIS_VehicleGroupAffected_t*) calloc(1, sizeof(ITIS_VehicleGroupAffected_t));
	*affectedVehCnt = 9217;
	asn_sequence_add(&rsmAffectedVehicles->list.array, affectedVehCnt);
	eventInfo->affectedVehicles = rsmAffectedVehicles;

	commonContainer->eventInfo = *eventInfo;


	// Region info
	auto regionInfo = (CommonContainer::CommonContainer__regionInfo*) calloc(1, sizeof(CommonContainer::CommonContainer__regionInfo));
	auto regionInfoCnt = (RegionInfo_t*) calloc(1, sizeof(RegionInfo_t));
	regionInfoCnt->referencePoint.lat = 423010836;
	regionInfoCnt->referencePoint.Long = -836990707;
	auto elev = (Common_Elevation_t*) calloc(1, sizeof(Common_Elevation_t));
	*elev = 2380;
	regionInfoCnt->referencePoint.elevation = elev;

	auto approachRegion = (AreaType_t*) calloc(1, sizeof(AreaType_t));
	approachRegion->present = AreaType_PR_broadRegion;
	approachRegion->choice.broadRegion.applicableHeading.heading = 0;
	approachRegion->choice.broadRegion.applicableHeading.tolerance = 5;
	auto broad = (BroadRegionArea_t*) calloc(1, sizeof(BroadRegionArea_t));
	broad->present = BroadRegionArea_PR_polygon;
	auto node1 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	node1->present = NodePointLLE_PR_node_3Doffset;
	node1->choice.node_3Doffset.lat_offset = 174;
	node1->choice.node_3Doffset.long_offset = -198;
	auto node2 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	node2->present = NodePointLLE_PR_node_3Doffset;
	node2->choice.node_3Doffset.lat_offset = 174;
	node2->choice.node_3Doffset.long_offset = 177;
	auto node3 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	node3->present = NodePointLLE_PR_node_3Doffset;
	node3->choice.node_3Doffset.lat_offset = -69;
	node3->choice.node_3Doffset.long_offset = 217;
	auto node4 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	node4->present = NodePointLLE_PR_node_3Doffset;
	node4->choice.node_3Doffset.lat_offset = -292;
	node4->choice.node_3Doffset.long_offset = 419;
	auto node5 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	node5->present = NodePointLLE_PR_node_3Doffset;
	node5->choice.node_3Doffset.lat_offset = -396;
	node5->choice.node_3Doffset.long_offset = 546;
	auto node6 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	node6->present = NodePointLLE_PR_node_3Doffset;
	node6->choice.node_3Doffset.lat_offset = -396;
	node6->choice.node_3Doffset.long_offset = -212;
	asn_sequence_add(&broad->choice.polygon.list.array, node1);
	asn_sequence_add(&broad->choice.polygon.list.array, node2);
	asn_sequence_add(&broad->choice.polygon.list.array, node3);
	asn_sequence_add(&broad->choice.polygon.list.array, node4);
	asn_sequence_add(&broad->choice.polygon.list.array, node5);
	asn_sequence_add(&broad->choice.polygon.list.array, node6);
	approachRegion->choice.broadRegion.broadArea = *broad;
	regionInfoCnt->approachRegion = approachRegion;

	asn_sequence_add(&regionInfo->list.array, regionInfoCnt);
	commonContainer->regionInfo = *regionInfo;
	message->commonContainer = *commonContainer;

	/// Begin Container Content
	auto content = (RoadSafetyMessage::RoadSafetyMessage__content*) calloc(1, sizeof(RoadSafetyMessage::RoadSafetyMessage__content));
	auto contentCnt = (ContentContainer_t*) calloc(1, sizeof(ContentContainer_t));

	// // Reduced Speed Zone Content
	// contentCnt->present = ContentContainer_PR_rszContainer;

	// auto reducedSpeedCnt = (ReducedSpeedZoneContainer_t*) calloc(1, sizeof(ReducedSpeedZoneContainer_t));
	// reducedSpeedCnt->speedLimit.type = 3;
	// reducedSpeedCnt->speedLimit.speed = 20;

	// auto rszReg = (RegionInfo_t*) calloc(1, sizeof(RegionInfo_t));
	// auto refPointRsz = (Position3D_t*) calloc(1, sizeof(Position3D_t));
	// refPointRsz->lat = 423010836;
	// refPointRsz->Long = -836990707;
	// auto elevRsz = (Common_Elevation_t*) calloc(1, sizeof(Common_Elevation_t));
	// *elevRsz = 2380;
	// refPointRsz->elevation = elevRsz;
	// rszReg->referencePoint = *refPointRsz;
	// auto refPointType = (ReferencePointType_t*) calloc(1, sizeof(ReferencePointType_t));
	// *refPointType = 0;
	// rszReg->referencePointType = refPointType;

	// auto rszApproachReg = (AreaType_t*) calloc(1, sizeof(AreaType_t));
	// rszApproachReg->present = AreaType_PR_paths;
	// auto pathList = (PathList_t*) calloc(1, sizeof(PathList_t));
	// auto rszPath = (Path_t*) calloc(1, sizeof(Path_t));
	// rszPath->pathWidth = 26;
	// auto pathNode1 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode1->present = NodePointLLE_PR_node_3Doffset;
	// pathNode1->choice.node_3Doffset.lat_offset = 14;
	// pathNode1->choice.node_3Doffset.long_offset = 23;
	// auto pathNode2 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode2->present = NodePointLLE_PR_node_3Doffset;
	// pathNode2->choice.node_3Doffset.lat_offset = 1324;
	// pathNode2->choice.node_3Doffset.long_offset = 50;
	// auto pathNode3 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode3->present = NodePointLLE_PR_node_3Doffset;
	// pathNode3->choice.node_3Doffset.lat_offset = 1945;
	// pathNode3->choice.node_3Doffset.long_offset = 131;
	// auto node3elevOff = (ElevOffset_t*) calloc(1, sizeof(ElevOffset_t));
	// *node3elevOff = 10;
	// pathNode3->choice.node_3Doffset.elev_offset = node3elevOff;
	// auto pathNode4 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode4->present = NodePointLLE_PR_node_3Doffset;
	// pathNode4->choice.node_3Doffset.lat_offset = 2645;
	// pathNode4->choice.node_3Doffset.long_offset = 483;
	// auto pathNode5 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode5->present = NodePointLLE_PR_node_3Doffset;
	// pathNode5->choice.node_3Doffset.lat_offset = 3095;
	// pathNode5->choice.node_3Doffset.long_offset = 956;
	// auto pathNode6 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode6->present = NodePointLLE_PR_node_3Doffset;
	// pathNode6->choice.node_3Doffset.lat_offset = 3475;
	// pathNode6->choice.node_3Doffset.long_offset = 1538;
	// auto pathNode7 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode7->present = NodePointLLE_PR_node_3Doffset;
	// pathNode7->choice.node_3Doffset.lat_offset = 3685;
	// pathNode7->choice.node_3Doffset.long_offset = 2024;
	// asn_sequence_add(&rszPath->pathPoints.list.array, pathNode1);
	// asn_sequence_add(&rszPath->pathPoints.list.array, pathNode2);
	// asn_sequence_add(&rszPath->pathPoints.list.array, pathNode3);
	// asn_sequence_add(&rszPath->pathPoints.list.array, pathNode4);
	// asn_sequence_add(&rszPath->pathPoints.list.array, pathNode5);
	// asn_sequence_add(&rszPath->pathPoints.list.array, pathNode6);
	// asn_sequence_add(&rszPath->pathPoints.list.array, pathNode7);
	// asn_sequence_add(&pathList->list.array, rszPath);
	// rszApproachReg->choice.paths = *pathList;
	// rszReg->approachRegion = rszApproachReg;
	// reducedSpeedCnt->rszRegion = *rszReg;


	// // Dynamic Content
	// contentCnt->present = ContentContainer_PR_dynamicInfoContainer;

	// auto dynamicInfoContainer = (DynamicInfoContainer_t*) calloc(1, sizeof(DynamicInfoContainer_t));
	// dynamicInfoContainer->priority = 3;
	// auto dmsString = (DynamicInfoContainer::DynamicInfoContainer__dmsSignString*) calloc(1, sizeof(DynamicInfoContainer::DynamicInfoContainer__dmsSignString));
	// auto myString = (ShortString_t*) calloc(1, sizeof(ShortString_t));
	// char* my_str = (char *) "Wrong Way Driver";
	// uint8_t* my_bytes = reinterpret_cast<uint8_t *>(my_str);
	// myString->buf = my_bytes;
	// myString->size = strlen(my_str);
	// asn_sequence_add(&dmsString->list.array, myString);
	// dynamicInfoContainer->dmsSignString = *dmsString;
	// dynamicInfoContainer->applicableRegion.referencePoint.lat = 389549610;
	// dynamicInfoContainer->applicableRegion.referencePoint.Long = -771493030;
	// auto appElev = (Common_Elevation_t*) calloc(1, sizeof(Common_Elevation_t));
	// *appElev = 390;
	// dynamicInfoContainer->applicableRegion.referencePoint.elevation = appElev;

	// // Curve Content
	// contentCnt->present = ContentContainer_PR_curveContainer;

	// auto curveContainer = (CurveContainer_t*) calloc(1, sizeof(CurveContainer_t));
	// curveContainer->advisorySpeed = 112;
	// auto curveRegion = (RegionInfo_t*) calloc(1, sizeof(RegionInfo_t));
	// auto refPointCurve = (Position3D_t*) calloc(1, sizeof(Position3D_t));
	// refPointCurve->lat = 423010836;
	// refPointCurve->Long = -836990707;
	// auto elevCurve = (Common_Elevation_t*) calloc(1, sizeof(Common_Elevation_t));
	// *elevCurve = 2380;
	// refPointCurve->elevation = elevCurve;
	// curveRegion->referencePoint = *refPointCurve;
	// auto refPointType = (ReferencePointType_t*) calloc(1, sizeof(ReferencePointType_t));
	// *refPointType = 0;
	// curveRegion->referencePointType = refPointType;

	// auto curveApproachReg = (AreaType_t*) calloc(1, sizeof(AreaType_t));
	// curveApproachReg->present = AreaType_PR_paths;
	// auto pathList = (PathList_t*) calloc(1, sizeof(PathList_t));
	// auto curvePath = (Path_t*) calloc(1, sizeof(Path_t));
	// curvePath->pathWidth = 26;
	// auto pathNode1 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode1->present = NodePointLLE_PR_node_3Doffset;
	// pathNode1->choice.node_3Doffset.lat_offset = 14;
	// pathNode1->choice.node_3Doffset.long_offset = 23;
	// auto pathNode2 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode2->present = NodePointLLE_PR_node_3Doffset;
	// pathNode2->choice.node_3Doffset.lat_offset = 1324;
	// pathNode2->choice.node_3Doffset.long_offset = 50;
	// auto pathNode3 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode3->present = NodePointLLE_PR_node_3Doffset;
	// pathNode3->choice.node_3Doffset.lat_offset = 1945;
	// pathNode3->choice.node_3Doffset.long_offset = 131;
	// auto node3elevOff = (ElevOffset_t*) calloc(1, sizeof(ElevOffset_t));
	// *node3elevOff = 10;
	// pathNode3->choice.node_3Doffset.elev_offset = node3elevOff;
	// auto pathNode4 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode4->present = NodePointLLE_PR_node_3Doffset;
	// pathNode4->choice.node_3Doffset.lat_offset = 2645;
	// pathNode4->choice.node_3Doffset.long_offset = 483;
	// auto pathNode5 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode5->present = NodePointLLE_PR_node_3Doffset;
	// pathNode5->choice.node_3Doffset.lat_offset = 3095;
	// pathNode5->choice.node_3Doffset.long_offset = 956;
	// auto pathNode6 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode6->present = NodePointLLE_PR_node_3Doffset;
	// pathNode6->choice.node_3Doffset.lat_offset = 3475;
	// pathNode6->choice.node_3Doffset.long_offset = 1538;
	// auto pathNode7 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode7->present = NodePointLLE_PR_node_3Doffset;
	// pathNode7->choice.node_3Doffset.lat_offset = 3685;
	// pathNode7->choice.node_3Doffset.long_offset = 2024;
	// asn_sequence_add(&curvePath->pathPoints.list.array, pathNode1);
	// asn_sequence_add(&curvePath->pathPoints.list.array, pathNode2);
	// asn_sequence_add(&curvePath->pathPoints.list.array, pathNode3);
	// asn_sequence_add(&curvePath->pathPoints.list.array, pathNode4);
	// asn_sequence_add(&curvePath->pathPoints.list.array, pathNode5);
	// asn_sequence_add(&curvePath->pathPoints.list.array, pathNode6);
	// asn_sequence_add(&curvePath->pathPoints.list.array, pathNode7);
	// asn_sequence_add(&pathList->list.array, curvePath);
	// curveApproachReg->choice.paths = *pathList;
	// curveRegion->approachRegion = curveApproachReg;
	// curveContainer->curveRegion = curveRegion;


	// // Incidents Content
	// contentCnt->present = ContentContainer_PR_incidentsContainer;

	// auto incidentsCnt = (IncidentsContainer_t*) calloc(1, sizeof(IncidentsContainer_t));
	// auto responderType = (IncidentsContainer_t::IncidentsContainer__responderType*) calloc(1, sizeof(IncidentsContainer_t::IncidentsContainer__responderType));
	// auto resp1 = (ITIS_ResponderGroupAffected_t*) calloc(1, sizeof(ITIS_ResponderGroupAffected_t));
	// *resp1 = 9733;
	// auto resp2 = (ITIS_ResponderGroupAffected_t*) calloc(1, sizeof(ITIS_ResponderGroupAffected_t));
	// *resp2 = 9734;
	// asn_sequence_add(&responderType->list.array, resp1);
	// asn_sequence_add(&responderType->list.array, resp2);
	// incidentsCnt->responderType = responderType;

	// auto incRegion = (RegionInfo_t*) calloc(1, sizeof(RegionInfo_t));
	// auto refPointInc = (Position3D_t*) calloc(1, sizeof(Position3D_t));
	// refPointInc->lat = 423010836;
	// refPointInc->Long = -836990707;
	// auto elevInc = (Common_Elevation_t*) calloc(1, sizeof(Common_Elevation_t));
	// *elevInc = 2380;
	// refPointInc->elevation = elevInc;
	// incRegion->referencePoint = *refPointInc;
	// auto refPointType = (ReferencePointType_t*) calloc(1, sizeof(ReferencePointType_t));
	// *refPointType = 0;
	// incRegion->referencePointType = refPointType;

	// auto incApproachReg = (AreaType_t*) calloc(1, sizeof(AreaType_t));
	// incApproachReg->present = AreaType_PR_paths;
	// auto pathList = (PathList_t*) calloc(1, sizeof(PathList_t));
	// auto incPath = (Path_t*) calloc(1, sizeof(Path_t));
	// incPath->pathWidth = 26;
	// auto pathNode1 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode1->present = NodePointLLE_PR_node_3Doffset;
	// pathNode1->choice.node_3Doffset.lat_offset = 14;
	// pathNode1->choice.node_3Doffset.long_offset = 23;
	// auto pathNode2 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	// pathNode2->present = NodePointLLE_PR_node_3Doffset;
	// pathNode2->choice.node_3Doffset.lat_offset = 1324;
	// pathNode2->choice.node_3Doffset.long_offset = 50;
	// asn_sequence_add(&incPath->pathPoints.list.array, pathNode1);
	// asn_sequence_add(&incPath->pathPoints.list.array, pathNode2);
	// asn_sequence_add(&pathList->list.array, incPath);
	// incApproachReg->choice.paths = *pathList;
	// incRegion->approachRegion = incApproachReg;
	// incidentsCnt->incidentLocation = *incRegion;


	// Situational Content
	contentCnt->present = ContentContainer_PR_situationalContainer;

	auto situationalCnt = (SituationalContainer_t*) calloc(1, sizeof(SituationalContainer_t));
	auto obstr = (Obstructions_t*) calloc(1, sizeof(Obstructions_t));
	obstr->location.lat = 423010836;
	obstr->location.Long = -836990707;
	auto obstrElev = (Common_Elevation_t*) calloc(1, sizeof(Common_Elevation_t));
	*obstrElev = 2380;
	obstr->location.elevation = obstrElev;
	auto desc = (ITIS_ITIScodes_t*) calloc(1, sizeof(ITIS_ITIScodes_t));
	*desc = 532;
	obstr->description = desc;
	situationalCnt->obstructions = obstr;

	auto obstrRegion = (RegionInfo_t*) calloc(1, sizeof(RegionInfo_t));
	auto refPointobstr = (Position3D_t*) calloc(1, sizeof(Position3D_t));
	refPointobstr->lat = 423010836;
	refPointobstr->Long = -836990707;
	auto elevobstr = (Common_Elevation_t*) calloc(1, sizeof(Common_Elevation_t));
	*elevobstr = 2380;
	refPointobstr->elevation = elevobstr;
	obstrRegion->referencePoint = *refPointobstr;
	obstrRegion->referencePoint = *refPointobstr;
	auto refPointType = (ReferencePointType_t*) calloc(1, sizeof(ReferencePointType_t));
	*refPointType = 0;
	obstrRegion->referencePointType = refPointType;

	auto obstrApproachReg = (AreaType_t*) calloc(1, sizeof(AreaType_t));
	obstrApproachReg->present = AreaType_PR_paths;
	auto pathList = (PathList_t*) calloc(1, sizeof(PathList_t));
	auto obstrPath = (Path_t*) calloc(1, sizeof(Path_t));
	obstrPath->pathWidth = 26;
	auto pathNode1 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode1->present = NodePointLLE_PR_node_3Doffset;
	pathNode1->choice.node_3Doffset.lat_offset = 14;
	pathNode1->choice.node_3Doffset.long_offset = 23;
	auto pathNode2 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode2->present = NodePointLLE_PR_node_3Doffset;
	pathNode2->choice.node_3Doffset.lat_offset = 1324;
	pathNode2->choice.node_3Doffset.long_offset = 50;
	asn_sequence_add(&obstrPath->pathPoints.list.array, pathNode1);
	asn_sequence_add(&obstrPath->pathPoints.list.array, pathNode2);
	asn_sequence_add(&pathList->list.array, obstrPath);
	obstrApproachReg->choice.paths = *pathList;
	obstrRegion->approachRegion = obstrApproachReg;
	situationalCnt->applicableRegion = *obstrRegion;


	// contentCnt->choice.rszContainer = *reducedSpeedCnt;
	// contentCnt->choice.dynamicInfoContainer = *dynamicInfoContainer;
	// contentCnt->choice.curveContainer = *curveContainer;
	// contentCnt->choice.incidentsContainer = *incidentsCnt;
	contentCnt->choice.situationalContainer = *situationalCnt;
	asn_sequence_add(&content->list.array, contentCnt);
	message->content = *content;

	xer_fprint(stdout, &asn_DEF_RoadSafetyMessage, message);

	//Encode RSM 
	tmx::messages::RsmEncodedMessage RsmEncodeMessage;
	auto _rsmMessage = new tmx::messages::RsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_rsmMessage->get_j2735_data());
	RsmEncodeMessage.set_data(TmxJ2735EncodedMessage<RoadSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
	free(message);
	free(frame_msg.get_j2735_data().get());
	std::cout << RsmEncodeMessage.get_payload_str() << std::endl;
	ASSERT_EQ(33,  RsmEncodeMessage.get_msgId());	
	std::string expectedRSMEncHex = "0021790f00802060c020218181431f9fa15ac000003f3f42c5800000001fc6c7d68000054edb8b1439665b0c194c0000281a40ae7e74902ba058a3fbb81b28fb720d1a3e7484448f9d1f960413812a76dc58a1cb32d860ca60800d054801d005d2296403258f33020e014929560f1a4c1787789364e30124e658fd00";
	ASSERT_EQ(expectedRSMEncHex, RsmEncodeMessage.get_payload_str());

	//Decode RSM
	auto rsm_ptr = RsmEncodeMessage.decode_j2735_message().get_j2735_data();
	ASSERT_EQ(12, rsm_ptr->commonContainer.eventInfo.eventUpdate);
}
	
TEST_F(J2735MessageTest, EncodeTrafficControlRequest){
	string tsm4str="<TestMessage04><body><tcrV01><reqid>C7C9A13FE6AC464E</reqid><reqseq>0</reqseq><scale>0</scale><bounds><TrafficControlBounds><oldest>27493419</oldest><reflon>-818349472</reflon><reflat>281118677</reflat><offsets><OffsetPoint><deltax>376</deltax><deltay>0</deltay></OffsetPoint><OffsetPoint><deltax>376</deltax><deltay>1320</deltay></OffsetPoint><OffsetPoint><deltax>0</deltax><deltay>1320</deltay></OffsetPoint></offsets></TrafficControlBounds></bounds></tcrV01> </body></TestMessage04>";
	std::stringstream ss;
	tsm4Message tsm4msg;
	tsm4EncodedMessage tsm4Enc;
	tmx::message_container_type container;
	ss<<tsm4str;
	container.load<XML>(ss);
	tsm4msg.set_contents(container.get_storage().get_tree());
	tsm4Enc.encode_j2735_message(tsm4msg);
	std::cout << tsm4Enc.get_payload_str()<<std::endl;
	ASSERT_EQ(244,  tsm4Enc.get_msgId());
}


TEST_F(J2735MessageTest, EncodeTrafficControlMessage){
	//Has <refwidth> tag in TCM
	string tsm5str="<TestMessage05><body><tcmV01><reqid>30642B129B984162</reqid><reqseq>0</reqseq><msgtot>9</msgtot><msgnum>9</msgnum><id>0034b8d88d084ffdaf23837926031658</id><updated>0</updated><package><label>workzone-laneclosed</label><tcids><Id128b>0034b8d88d084ffdaf23837926031658</Id128b></tcids></package><params><vclasses><micromobile/><motorcycle/><passenger-car/><light-truck-van/><bus/><two-axle-six-tire-single-unit-truck/><three-axle-single-unit-truck/><four-or-more-axle-single-unit-truck/><four-or-fewer-axle-single-trailer-truck/><five-axle-single-trailer-truck/><six-or-more-axle-single-trailer-truck/><five-or-fewer-axle-multi-trailer-truck/><six-axle-multi-trailer-truck/><seven-or-more-axle-multi-trailer-truck/></vclasses><schedule><start>27506547</start><end>153722867280912</end><dow>1111111</dow></schedule><regulatory><true/></regulatory><detail><closed><notopen/></closed></detail></params><geometry><proj>epsg:3785</proj><datum>WGS84</datum><reftime>27506547</reftime><reflon>-818331529</reflon><reflat>281182119</reflat><refelv>0</refelv><refwidth>424</refwidth><heading>3403</heading><nodes><PathNode><x>0</x><y>0</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>721</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-204</x><y>722</y><width>2</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>-2</width></PathNode><PathNode><x>-203</x><y>721</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-203</x><y>722</y><width>0</width></PathNode><PathNode><x>-13</x><y>46</y><width>0</width></PathNode></nodes></geometry></tcmV01></body></TestMessage05>";
	std::stringstream ss;
	tsm5Message tsm5msg;
	tsm5EncodedMessage tsm5Enc;
	tmx::message_container_type container;
	ss<<tsm5str;
	container.load<XML>(ss);
	tsm5msg.set_contents(container.get_storage().get_tree());
	tsm5Enc.encode_j2735_message(tsm5msg);
	std::cout << tsm5Enc.get_payload_str()<<std::endl;
	ASSERT_EQ(245,  tsm5Enc.get_msgId());	

	//No <refwidth> tag in TCM
	tsm5str="<TestMessage05><body><tcmV01><reqid>D0E0C6E650394C06</reqid><reqseq>0</reqseq><msgtot>1</msgtot><msgnum>1</msgnum><id>002740591d261d2e99e477df0a82db26</id><updated>0</updated><package><label>workzone</label><tcids><Id128b>002740591d261d2e99e477df0a82db26</Id128b></tcids></package><params><vclasses><micromobile/><motorcycle/><passenger-car/><light-truck-van/><bus/><two-axle-six-tire-single-unit-truck/><three-axle-single-unit-truck/><four-or-more-axle-single-unit-truck/><four-or-fewer-axle-single-trailer-truck/><five-axle-single-trailer-truck/><six-or-more-axle-single-trailer-truck/><five-or-fewer-axle-multi-trailer-truck/><six-axle-multi-trailer-truck/><seven-or-more-axle-multi-trailer-truck/></vclasses><schedule><start>27777312</start><end>153722867280912</end><dow>1111111</dow></schedule><regulatory><true/></regulatory><detail><closed><notopen/></closed></detail></params><geometry><proj>epsg:3785</proj><datum>WGS84</datum><reftime>27777312</reftime><reflon>-771483519</reflon><reflat>389549109</reflat><refelv>0</refelv><heading>3312</heading><nodes><PathNode><x>1</x><y>0</y><width>0</width></PathNode><PathNode><x>-1498</x><y>-26</y><width>2</width></PathNode><PathNode><x>-1497</x><y>45</y><width>7</width></PathNode><PathNode><x>-1497</x><y>91</y><width>11</width></PathNode><PathNode><x>-370</x><y>34</y><width>2</width></PathNode></nodes></geometry></tcmV01></body></TestMessage05>";
	ss<<tsm5str;
	container.load<XML>(ss);
	tsm5msg.set_contents(container.get_storage().get_tree());
	tsm5Enc.encode_j2735_message(tsm5msg);
	std::cout << tsm5Enc.get_payload_str()<<std::endl;
	ASSERT_EQ(245,  tsm5Enc.get_msgId());		
}

TEST_F (J2735MessageTest, EncodeSrm)
{
	SignalRequestMessage_t *message = (SignalRequestMessage_t *)calloc(1, sizeof(SignalRequestMessage_t));
	message->second = 12;
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
	#if SAEJ2735_SPEC < 2020
	DSRC_Angle_t *heading_angle = (DSRC_Angle_t *)calloc(1, sizeof(DSRC_Angle_t));
	#else
	Common_Angle_t *heading_angle = (Common_Angle_t *)calloc(1, sizeof(Common_Angle_t));
	#endif
	*heading_angle = 123;
	position->heading = heading_angle;
	Position3D_t *position_point = (Position3D_t *)calloc(1, sizeof(Position3D_t));
	#if SAEJ2735_SPEC < 2020
	DSRC_Elevation_t *elev = (DSRC_Elevation_t *)calloc(1, sizeof(DSRC_Elevation_t));
	#else
	Common_Elevation_t *elev = (Common_Elevation_t *)calloc(1, sizeof(Common_Elevation_t));
	#endif
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
	message->requestor = *requestor;

	SignalRequestList_t *requests = (SignalRequestList_t *)calloc(1, sizeof(SignalRequestList_t));
	//First: Request Package
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

	//Second: Request Package
	SignalRequestPackage_t *request_package_2 = (SignalRequestPackage_t *)calloc(1, sizeof(SignalRequestPackage_t));
	request_package_2->minute = min;
	request_package_2->duration = duration;
	request_package_2->second = second;
	SignalRequest_t *request_2 = (SignalRequest_t *)calloc(1, sizeof(SignalRequest_t));
	IntersectionReferenceID_t *referId2 = (IntersectionReferenceID_t *)calloc(1, sizeof(IntersectionReferenceID_t));
	referId2->id = 2333;
	request_2->id = *referId2;
	request_2->requestID = 2;
	request_2->requestType = 1;
	IntersectionAccessPoint_t *inBoundLane2 = (IntersectionAccessPoint_t *)calloc(1, sizeof(IntersectionAccessPoint_t));
	inBoundLane2->present = IntersectionAccessPoint_PR_approach;
	inBoundLane2->choice.approach = 1;
	request_2->inBoundLane = *inBoundLane2;
	request_package_2->request = *request_2;
	asn_sequence_add(&requests->list.array, request_package_2);
	message->requests = requests;
	tmx::messages::SrmEncodedMessage srmEncodeMessage;
	auto _srmMessage = new tmx::messages::SrmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_srmMessage->get_j2735_data());
	srmEncodeMessage.set_data(TmxJ2735EncodedMessage<SignalRequestMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
	free(message);
	free(frame_msg.get_j2735_data().get());
	ASSERT_EQ(29,  srmEncodeMessage.get_msgId());	
	std::string expectedSRMEncHex = "001d311000605c0098c020008003d825e003d380247408910007b04bc007a60004303028001a6bbb1c9ad7882858201801ef8028";
	ASSERT_EQ(expectedSRMEncHex, srmEncodeMessage.get_payload_str());	
}

TEST_F(J2735MessageTest, EncodeTravelerInformation){
	//Advisory
	#if SAEJ2735_SPEC < 2020
	string timStr="<TravelerInformation><msgCnt>1</msgCnt><timeStamp>115549</timeStamp><packetID>000000000023667BAC</packetID><dataFrames><TravelerDataFrame><sspTimRights>0</sspTimRights><frameType><advisory/></frameType><msgId><roadSignID><position><lat>389549775</lat><long>-771491835</long><elevation>390</elevation></position><viewAngle>1111111111111111</viewAngle><mutcdCode><warning/></mutcdCode></roadSignID></msgId><startTime>115549</startTime><duratonTime>1</duratonTime><priority>7</priority><sspLocationRights>0</sspLocationRights><regions><GeographicalPath><anchor><lat>389549775</lat><long>-771491835</long><elevation>390</elevation></anchor><directionality><both/></directionality><closedPath><true/></closedPath><description><geometry><direction>1111111111111111</direction><circle><center><lat>389549775</lat><long>-771491835</long><elevation>390</elevation></center><radius>74</radius><units><meter/></units></circle></geometry></description></GeographicalPath></regions><sspMsgRights1>0</sspMsgRights1><sspMsgRights2>0</sspMsgRights2><content><advisory><SEQUENCE><item><itis>7186</itis></item></SEQUENCE><SEQUENCE><item><text>curve</text></item></SEQUENCE><SEQUENCE><item><itis>13569</itis></item></SEQUENCE></advisory></content><url>987654321</url></TravelerDataFrame></dataFrames></TravelerInformation>";
	#else
	string timStr="<TravelerInformation><msgCnt>1</msgCnt><packetID>00000000000F9E1D8D</packetID><dataFrames><TravelerDataFrame><notUsed>0</notUsed><frameType><unknown/></frameType><msgId><roadSignID><position><lat>389549153</lat><long>-771488965</long><elevation>400</elevation></position><viewAngle>0000000000000000</viewAngle><mutcdCode><none/></mutcdCode></roadSignID></msgId><startYear>2023</startYear><startTime>394574</startTime><durationTime>32000</durationTime><priority>5</priority><notUsed1>0</notUsed1><regions><GeographicalPath><anchor><lat>389549153</lat><long>-771488965</long><elevation>400</elevation></anchor><laneWidth>366</laneWidth><directionality><forward/></directionality><closedPath><false/></closedPath><direction>0000000000000000</direction><description><path><offset><xy><nodes><NodeXY><delta><node-LatLon><lon>-771489394</lon><lat>389549194</lat></node-LatLon></delta><attributes><dElevation>-10</dElevation></attributes></NodeXY><NodeXY><delta><node-LatLon><lon>-771487215</lon><lat>389548996</lat></node-LatLon></delta><attributes><dElevation>10</dElevation></attributes></NodeXY><NodeXY><delta><node-LatLon><lon>-771485210</lon><lat>389548981</lat></node-LatLon></delta><attributes><dElevation>10</dElevation></attributes></NodeXY></nodes></xy></offset></path></description></GeographicalPath></regions><notUsed2>0</notUsed2><notUsed3>0</notUsed3><content><speedLimit><SEQUENCE><item><itis>27</itis></item></SEQUENCE><SEQUENCE><item><text>Curve Ahead</text></item></SEQUENCE><SEQUENCE><item><itis>2564</itis></item></SEQUENCE><SEQUENCE><item><text>25</text></item></SEQUENCE><SEQUENCE><item><itis>8720</itis></item></SEQUENCE></speedLimit></content></TravelerDataFrame></dataFrames></TravelerInformation>";
	#endif
	std::stringstream ss;
	TimMessage timMsg;
	TimEncodedMessage timEnc;
	tmx::message_container_type container;
	ss<<timStr;
	container.load<XML>(ss);
	timMsg.set_contents(container.get_storage().get_tree());
	timEnc.encode_j2735_message(timMsg);
	ASSERT_EQ(31,  timEnc.get_msgId());	
	#if SAEJ2735_SPEC < 2020
	string expectedHex = "001f526011c35d000000000023667bac0407299b9ef9e7a9b9408230dfffe4386ba00078005a53373df3cf5372810461b90ffff53373df3cf53728104618129800010704a04c7d7976ca3501872e1bb66ad19b2620";
	#else
	string expectedHex = "001f6820100000000000f9e1d8d0803299b9eac27a9baa74232000000fcec0a9df4028007e53373d584f53754e846400b720000000b8f5374e3666e7ac5013ece3d4ddc1099b9e988050538f5378f9666e7a5a814140034000dea1f5e5db2a083a32e1c80a048b26a22100";
	#endif
	ASSERT_EQ(expectedHex, timEnc.get_payload_str());			
}

TEST_F(J2735MessageTest, EncodeSDSM)
{
	auto message = (SensorDataSharingMessage_t*)calloc(1, sizeof(SensorDataSharingMessage_t));
	message->msgCnt = 10;
	uint8_t my_bytes_id[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
	message->sourceID.buf = my_bytes_id;
	message->sourceID.size = sizeof(my_bytes_id);
	message->equipmentType = EquipmentType_rsu;
	

	auto sDSMTimeStamp = (DDateTime_t*) calloc(1, sizeof(DDateTime_t));
	auto year = (DYear_t*) calloc(1, sizeof(DYear_t));
	*year= 2024;
	sDSMTimeStamp->year = year;
	message->sDSMTimeStamp = *sDSMTimeStamp;

	message->refPos.lat = 423010070;
	message->refPos.Long = -836987660;
	auto elevObj = (Common_Elevation_t*) calloc(1, sizeof(Common_Elevation_t));
	*elevObj = 2380;
	message->refPos.elevation = elevObj;

	message->refPosXYConf.orientation = 10;
	message->refPosXYConf.semiMajor = 12;
	message->refPosXYConf.semiMinor = 52;

	auto objects = (DetectedObjectList_t*) calloc(1, sizeof(DetectedObjectList_t));
	auto objectData = (DetectedObjectData_t*) calloc(1, sizeof(DetectedObjectData_t));
	objectData->detObjCommon.objType = ObjectType_vru;
	objectData->detObjCommon.objTypeCfd = 98;
	objectData->detObjCommon.objectID = 8010;
	objectData->detObjCommon.measurementTime = 1;
	objectData->detObjCommon.timeConfidence = 8;
	objectData->detObjCommon.pos.offsetX = 1;
	objectData->detObjCommon.pos.offsetY = 1;
	objectData->detObjCommon.posConfidence.pos = 11;
	objectData->detObjCommon.posConfidence.elevation = 11;
	objectData->detObjCommon.speed = 1;
	objectData->detObjCommon.speedConfidence = 2;
	objectData->detObjCommon.heading = 0;
	objectData->detObjCommon.headingConf = 2;
	ASN_SEQUENCE_ADD(&objects->list.array, objectData);
	message->objects = *objects;
	xer_fprint(stdout, &asn_DEF_SensorDataSharingMessage, message);

	//Encode SDSM 
	tmx::messages::SdsmEncodedMessage SdsmEncodeMessage;
	auto _sdsmMessage = new tmx::messages::SdsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_sdsmMessage->get_j2735_data());
	SdsmEncodeMessage.set_data(TmxJ2735EncodedMessage<SensorDataSharingMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
	free(message);
	free(frame_msg.get_j2735_data().get());
	ASSERT_EQ(41,  SdsmEncodeMessage.get_msgId());	
	std::string expectedSDSMEncHex = "0029270a010c0c0a301fa14edb8816396666f3194c0c34000a00002c43e94bba4200020002ec00280002";
	ASSERT_EQ(expectedSDSMEncHex, SdsmEncodeMessage.get_payload_str());	

	//Decode SDSM
	auto sdsm_ptr = SdsmEncodeMessage.decode_j2735_message().get_j2735_data();
	ASSERT_EQ(10, sdsm_ptr->msgCnt);
}
}
