//============================================================================
// Name        : J2735MessageTest.cpp
// Description : Unit tests for the J2735 Message library.
//============================================================================

#include <boost/any.hpp>
#include <gtest/gtest.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <tmx/messages/message_document.hpp>
#include <vector>

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
		return MsgClass::get_default_msgId();
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
			msgTypes[api::basicSafetyMessageVerbose_D] = new msg_type_impl<BsmvMessage>();
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
			msgTypes[api::uperFrame_D] = new msg_type_impl<UperFrameMessage>();
			enumNames[api::personalMobilityMessage] = api::MSGSUBTYPE_PERSONALMOBILITYMESSAGE_STRING;
			msgTypes[api::personalMobilityMessage] = new msg_type_impl<PmmMessage>();
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

TEST_F(J2735MessageTest, FactoryTest) {
	static routeable_message *msgFromId = NULL;
	static routeable_message *msgFromName = NULL;
	static routeable_message *msgFromBytes = NULL;
	static msg_type *mType = NULL;

	// Loop through every known messages
	for (unsigned int i = 0; i < enumNames.size(); i++) {
		if (i == api::J2735 || enumNames[i].length() <= 0)
			continue;

		mType = msgTypes[i];
		cout << "Verifying " << enumNames[i] << endl;

		msgFromId = factory.NewMessage(i);										// Build a message object from the ID
		ASSERT_TRUE(msgFromId);													// Was the object built correctly?
		ASSERT_EQ(msgFromId->get_type(), enumNames[api::J2735]);  				// Is the type name correct?
		ASSERT_EQ(msgFromId->get_subtype(), enumNames[i]);						// Is the subtype name correct?
		ASSERT_EQ(msgFromId->get_encoding(), api::ENCODING_ASN1_BER_STRING);	// Is the encoding correct?

		ASSERT_TRUE(mType->set_message(msgFromId));								// Is this the correct type?
		ASSERT_EQ(mType->get_default_msgId(), i);								// Is this the correct default message id?

		msgFromName = factory.NewMessage(msgFromId->get_subtype());				// Build a message object from the known name
		ASSERT_TRUE(msgFromName);												// Was the object built correctly?
		ASSERT_EQ(msgFromName->get_type(), enumNames[api::J2735]);  			// Is the type name correct?
		ASSERT_EQ(msgFromName->get_subtype(), enumNames[i]);					// Is the subtype name correct?
		ASSERT_EQ(msgFromName->get_encoding(), api::ENCODING_ASN1_BER_STRING);	// Is the encoding correct?

		ASSERT_TRUE(mType->set_message(msgFromName));							// Is this the correct type?
		ASSERT_EQ(mType->get_default_msgId(), i);								// Is this the correct default message id?

		if (testBytes[i].length() > 0) {
			byte_stream bytes = attribute_lexical_cast<byte_stream>(testBytes[i]);
			msgFromBytes = factory.NewMessage(bytes);

			ASSERT_EQ(msgFromBytes->get_type(), enumNames[api::J2735]);  		// Is the type name correct?
			ASSERT_EQ(msgFromBytes->get_subtype(), enumNames[i]);				// Is the subtype name correct?
			ASSERT_EQ(msgFromBytes->get_encoding(), api::ENCODING_ASN1_BER_STRING);	// Is the encoding correct?
			ASSERT_EQ(msgFromBytes->get_payload_bytes(), bytes);				// Is the payload correct?

			ASSERT_TRUE(mType->set_message(msgFromBytes));						// Is this the correct type?
			ASSERT_EQ(mType->get_bytes(), bytes);								// Is the message data correct?
			ASSERT_EQ(mType->get_default_msgId(), i);							// Is this the correct default message id?
			ASSERT_EQ(mType->get_msgId(), i);									// Is this the correct message id?

			// Save off the underlying message
			string msgXml = mType->get_payload();

			// Build up a new message from scratch
			xml_message newMsg(msgXml);
			message_document msgDoc(newMsg);

			ASSERT_STRCASEEQ(mType->message_tag().c_str(), msgDoc.root().first_child().name());	// Is the XML tag correct?

			ASSERT_TRUE(mType->set_message(newMsg.to_string()));				// Is this the correct type?
			ASSERT_EQ(mType->get_bytes(), bytes);								// Is the message data correct?
			ASSERT_EQ(mType->get_default_msgId(), i);							// Is this the correct default message id?
			ASSERT_EQ(mType->get_msgId(), i);									// Is this the correct message id?
		}

		delete(msgFromBytes);
		msgFromBytes = NULL;
		delete(msgFromName);
		msgFromName = NULL;
		delete(msgFromId);
		msgFromId = NULL;
	}
}

}
