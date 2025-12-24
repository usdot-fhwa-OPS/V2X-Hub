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
#include <stol-j2735-201603-carma/jer_encoder.h>

using namespace std;
using namespace battelle::attributes;
using namespace tmx;
using namespace tmx::messages;

namespace unit_test {




	
TEST(J2735MessageTest, EncodeMobilityOperation)
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


TEST(J2735MessageTest, EncodeMobilityRequest)
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
	message->body.expiration = (MobilityTimestamp_t*)malloc(sizeof(MobilityTimestamp_t));
	message->body.expiration->buf = my_bytes_1;
	message->body.expiration->size = strlen(my_str_1);

	MobilityECEFOffset_t* offset = (MobilityECEFOffset_t*)calloc(1, sizeof(MobilityECEFOffset_t) );
	offset->offsetX = 1;
	offset->offsetY = 1;
	offset->offsetZ = 1;
	ASN_SEQUENCE_ADD(&message->body.trajectory->list.array, offset);
	ASN_SEQUENCE_ADD(&message->body.trajectory->list.array, offset);

	message->body.trajectoryStart = (MobilityLocation*)malloc(sizeof(MobilityLocation));
	message->body.trajectoryStart->ecefX = 1;
	message->body.trajectoryStart->ecefY = 1;
	message->body.trajectoryStart->ecefZ = 1;
	message->body.trajectoryStart->timestamp.buf = my_bytes_1;
	message->body.trajectoryStart->timestamp.size = strlen(my_str_1);
		
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


TEST(J2735MessageTest, EncodeMobilityResponse)
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


TEST(J2735MessageTest, EncodeBasicSafetyMessage)
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



TEST(J2735MessageTest, EncodeBasicSafetyMessage_PartII)
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
	jer_fprint(stdout, &asn_DEF_BasicSafetyMessage, message);
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
	std::cout << psmENC.get_payload_str()<<std::endl;
	ASSERT_EQ(32,  psmENC.get_msgId());
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
	std::cout << tsm4Enc.get_payload_str()<<std::endl;
	ASSERT_EQ(244,  tsm4Enc.get_msgId());
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
	std::cout << tsm5Enc.get_payload_str()<<std::endl;
	ASSERT_EQ(245,  tsm5Enc.get_msgId());	

	//No <refwidth> tag in TCM
	tsm5str="<TestMessage05><body><tcmV01><reqid>D0E0C6E650394C06</reqid><reqseq>0</reqseq><msgtot>1</msgtot><msgnum>1</msgnum><id>002740591d261d2e99e477df0a82db26</id><updated>0</updated><package><label>workzone</label><tcids><Id128b>002740591d261d2e99e477df0a82db26</Id128b></tcids></package><params><vclasses><micromobile/><motorcycle/><passenger-car/><light-truck-van/><bus/><two-axle-six-tire-single-unit-truck/><three-axle-single-unit-truck/><four-or-more-axle-single-unit-truck/><four-or-fewer-axle-single-trailer-truck/><five-axle-single-trailer-truck/><six-or-more-axle-single-trailer-truck/><five-or-fewer-axle-multi-trailer-truck/><six-axle-multi-trailer-truck/><seven-or-more-axle-multi-trailer-truck/></vclasses><schedule><start>27777312</start><end>153722867280912</end><dow>1111111</dow></schedule><regulatory><true/></regulatory><detail><closed><taperleft/></closed></detail></params><geometry><proj>epsg:3785</proj><datum>WGS84</datum><reftime>27777312</reftime><reflon>-771483519</reflon><reflat>389549109</reflat><refelv>0</refelv><heading>3312</heading><nodes><PathNode><x>1</x><y>0</y><width>0</width></PathNode><PathNode><x>-1498</x><y>-26</y><width>2</width></PathNode><PathNode><x>-1497</x><y>45</y><width>7</width></PathNode><PathNode><x>-1497</x><y>91</y><width>11</width></PathNode><PathNode><x>-370</x><y>34</y><width>2</width></PathNode></nodes></geometry></tcmV01></body></TestMessage05>";
	ss<<tsm5str;
	container.load<XML>(ss);
	tsm5msg.set_contents(container.get_storage().get_tree());
	tsm5Enc.encode_j2735_message(tsm5msg);
	std::cout << tsm5Enc.get_payload_str()<<std::endl;
	ASSERT_EQ(245,  tsm5Enc.get_msgId());		
}

TEST (J2735MessageTest, EncodeSrm)
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
	EXPECT_EQ(31,  timEnc.get_msgId());
	// 2024 version and up
	#if SAEJ2735_SPEC >= 2024
	// 2020 version
	#elif SAEJ2735_SPEC == 2020
	// 2016 version
	#else
	#endif
	EXPECT_EQ(expectedHex, timEnc.get_payload_str());			
}

TEST(J2735MessageTest, EncodeSDSM)
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

	message->refPos.lat = 38121212;
	message->refPos.Long = -77121212;

	message->refPosXYConf.orientation = 10;
	message->refPosXYConf.semiMajor = 12;
	message->refPosXYConf.semiMinor = 52;

	auto objects = (DetectedObjectList_t*) calloc(1, sizeof(DetectedObjectList_t));
	auto objectData = (DetectedObjectData_t*) calloc(1, sizeof(DetectedObjectData_t));
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
	ASN_SEQUENCE_ADD(&objects->list.array, objectData);
	message->objects = *objects;
	xer_fprint(stdout, &asn_DEF_SensorDataSharingMessage, message);

	//Encode SDSM 
	tmx::messages::SdsmEncodedMessage SdsmEncodeMessage;
	auto _sdsmMessage = new tmx::messages::SdsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_sdsmMessage->get_j2735_data());
	SdsmEncodeMessage.set_data(TmxJ2735EncodedMessage<SdsmMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
	free(message);
	free(frame_msg.get_j2735_data().get());
	ASSERT_EQ(41,  SdsmEncodeMessage.get_msgId());	
	std::string expectedSDSMEncHex = "0029250a010c0c0a101f9c37ea97fc66b10b430c34000a00000020002bba0a000200004400240009";
	ASSERT_EQ(expectedSDSMEncHex, SdsmEncodeMessage.get_payload_str());	

	//Decode SDSM
	auto sdsm_ptr = SdsmEncodeMessage.decode_j2735_message().get_j2735_data();
	ASSERT_EQ(10, sdsm_ptr->msgCnt);
}
}
