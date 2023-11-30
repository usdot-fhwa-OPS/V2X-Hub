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
			enumNames[api::personalMobilityMessage] = api::MSGSUBTYPE_PERSONALMOBILITYMESSAGE_STRING;
		//	msgTypes[api::personalMobilityMessage] = new msg_type_impl<PmmMessage>();
			testBytes[api::personalMobilityMessage] = "303a800111830200f58431482362c99e568d5b375b95c39c4b58b2c8cd6e168d5b2d68c9b366ad5a3460c1830000d693a401ad2747fc7e09b3720034";
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
	message->equipmentType = EquipmentType_unknown;
	

	auto sDSMTimeStamp = (DDateTime_t*) calloc(1, sizeof(DDateTime_t));
	auto year = (DYear_t*) calloc(1, sizeof(DYear_t));
	*year= 2023;
	sDSMTimeStamp->year = year;
	message->sDSMTimeStamp = *sDSMTimeStamp;

	message->refPos.lat = 38.121212;
	message->refPos.Long = -77.121212;

	message->refPosXYConf.orientation = 10;
	message->refPosXYConf.semiMajor = 12;
	message->refPosXYConf.semiMinor = 52;

	auto objects = (DetectedObjectList_t*) calloc(1, sizeof(DetectedObjectList_t));
	auto objectData = (DetectedObjectData_t*) calloc(1, sizeof(DetectedObjectData_t));
	objectData->detObjCommon.objType = ObjectType_unknown;
	objectData->detObjCommon.objTypeCfd = 1;
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
	ASN_SEQUENCE_ADD(&objects->list.array, objectData);
	message->objects = *objects;
	std::cout << message <<std::endl;
	xer_fprint(stdout, &asn_DEF_SensorDataSharingMessage, message);

	//Encode SDSM 
	tmx::messages::SdsmEncodedMessage SdsmEncodeMessage;
	auto _sdsmMessage = new tmx::messages::SdsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_sdsmMessage->get_j2735_data());
	SdsmEncodeMessage.set_data(TmxJ2735EncodedMessage<SignalRequestMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
	free(message);
	free(frame_msg.get_j2735_data().get());
	ASSERT_EQ(41,  SdsmEncodeMessage.get_msgId());	
	std::string expectedSDSMEncHex = "0029250a010c0c0a101f9c35a4e9266b49d1b20c34000a00000020000bba0a000200004400240009";
	ASSERT_EQ(expectedSDSMEncHex, SdsmEncodeMessage.get_payload_str());	

	//Decode SDSM
	auto sdsm_ptr = SdsmEncodeMessage.decode_j2735_message().get_j2735_data();
	ASSERT_EQ(10, sdsm_ptr->msgCnt);
}
}
