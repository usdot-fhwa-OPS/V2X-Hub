#if SAEJ2735_SPEC >= 2024
#include <boost/any.hpp>
#include <gtest/gtest.h>
#include <tmx/j2735_messages/RoadSafetyMessage.hpp>
#include <vector>
#include <iostream>
#include <chrono>
#include <sstream>

using namespace std;
using namespace tmx;
using namespace tmx::messages;

// Function to convert hex string to byte array
std::vector<uint8_t> hexStringToByteArray(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = (uint8_t) strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    return bytes;
}
// Test encoding Road Safety Message from XML
TEST(RoadSafetyMessageTest, EncodeRoadSafetyMessageXML)
{
       // Encode RSM XML
       string rsm="<RoadSafetyMessage> <commonContainer> <eventInfo> <eventID> <operatorID> <fullRdAuthID>1.0.15628.4.1.17.1</fullRdAuthID> </operatorID> <uniqueID>01 0C 0C 0A</uniqueID> </eventID> <eventUpdate>12</eventUpdate> <eventCancellation><false/></eventCancellation> <startDateTime> <year>2024</year> <month>3</month> <day>19</day> <hour>15</hour> <minute>30</minute> <second>45</second> </startDateTime> <eventRecurrence> <EventRecurrence> <monday><true/></monday> <tuesday><true/></tuesday> <wednesday><true/></wednesday> <thursday><true/></thursday> <friday><true/></friday> <saturday><true/></saturday> <sunday><true/></sunday> </EventRecurrence> </eventRecurrence> <causeCode>7</causeCode> <subCauseCode>1793</subCauseCode> <affectedVehicles><all-vehicles/> </affectedVehicles> </eventInfo> <regionInfo> <RegionInfo> <referencePoint> <lat>389549610</lat> <long>-771493030</long> <elevation>390</elevation> </referencePoint> </RegionInfo> </regionInfo> </commonContainer> <content> <dynamicInfoContainer> <priority><critical/></priority> <dmsSignString> <ShortString>Wrong Way Driver</ShortString> </dmsSignString> <applicableRegion> <referencePoint> <lat>389549610</lat> <long>-771493030</long> <elevation>390</elevation> </referencePoint> </applicableRegion> </dynamicInfoContainer> </content> </RoadSafetyMessage>";
       std::stringstream ss;
       RsmMessage rsmmessage;
       RsmEncodedMessage rsmENC;
       tmx::message_container_type container;
       ss<<rsm;
       container.load<XML>(ss);
       rsmmessage.set_contents(container.get_storage().get_tree());
       rsmENC.encode_j2735_message(rsmmessage);
       std::cout << rsmENC.get_payload_str() << std::endl;
       EXPECT_EQ(33,  rsmENC.get_msgId());
}
// Test encoding Road Safety Message with lane closure content
TEST(RoadSafetyMessageTest, EncodeRoadSafetyMessageLaneClosure) {

	/**
	* Populate RSM 
	*/      
	auto message = (RoadSafetyMessage_t*) calloc(1, sizeof(RoadSafetyMessage_t));
	auto commonContainer = (CommonContainer_t*) calloc(1, sizeof(CommonContainer_t));
	auto eventInfo = (EventInfo_t*) calloc(1, sizeof(EventInfo_t));

	// Event ID info
	auto eventID = (EventIdentifier_t*) calloc(1, sizeof(EventIdentifier_t));
	eventID->operatorID.present = RoadAuthorityID_PR_fullRdAuthID;

	// Road Authority OID
	uint32_t oid[] = {1,0,15628,4,1,17,1,0};
	int success = OBJECT_IDENTIFIER_set_arcs(&(eventID->operatorID.choice.fullRdAuthID), oid, sizeof(oid)/sizeof(oid[0]));
	if (success != 0) {
		std::cout << "Failed to set OID arcs" << std::endl;
		GTEST_FAIL();
	}
	uint8_t  my_bytes_uid[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
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

	// ITIS cause codes
	eventInfo->causeCode = 2;
	
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

	// Lane Closure Content
	contentCnt->present = ContentContainer_PR_laneClosureContainer;

	auto laneClosureCnt = (LaneClosureContainer_t*) calloc(1, sizeof(LaneClosureContainer_t));
	auto lnStatus = (LaneClosureContainer::LaneClosureContainer__laneStatus*) calloc(1, sizeof(LaneClosureContainer::LaneClosureContainer__laneStatus));
	auto laneInfo1 = (LaneInfo_t*) calloc(1, sizeof(LaneInfo_t));
	laneInfo1->lanePosition = 1;
	laneInfo1->laneClosed = true;
	asn_sequence_add(&lnStatus->list.array, laneInfo1);
	laneClosureCnt->laneStatus = lnStatus;

	auto closureReg = (RegionInfo_t*) calloc(1, sizeof(RegionInfo_t));
	auto refPointClosure = (Position3D_t*) calloc(1, sizeof(Position3D_t));
	refPointClosure->lat = 423010836;
	refPointClosure->Long = -836990707;
	auto elevClosure = (Common_Elevation_t*) calloc(1, sizeof(Common_Elevation_t));
	*elevClosure = 2380;
	refPointClosure->elevation = elevClosure;
	closureReg->referencePoint = *refPointClosure;
	auto refPointType = (ReferencePointType_t*) calloc(1, sizeof(ReferencePointType_t));
	*refPointType = 0;
	closureReg->referencePointType = refPointType;

	auto closureApproachReg = (AreaType_t*) calloc(1, sizeof(AreaType_t));
	closureApproachReg->present = AreaType_PR_paths;
	auto pathList = (PathList_t*) calloc(1, sizeof(PathList_t));
	auto closurePath = (Path_t*) calloc(1, sizeof(Path_t));
	closurePath->pathWidth = 26;
	auto pathNode1 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode1->present = NodePointLLE_PR_node_3Doffset;
	pathNode1->choice.node_3Doffset.lat_offset = 14;
	pathNode1->choice.node_3Doffset.long_offset = 23;
	auto pathNode2 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode2->present = NodePointLLE_PR_node_3Doffset;
	pathNode2->choice.node_3Doffset.lat_offset = 1324;
	pathNode2->choice.node_3Doffset.long_offset = 50;
	asn_sequence_add(&closurePath->pathPoints.list.array, pathNode1);
	asn_sequence_add(&closurePath->pathPoints.list.array, pathNode2);
	asn_sequence_add(&pathList->list.array, closurePath);
	closureApproachReg->choice.paths = *pathList;
	closureReg->approachRegion = closureApproachReg;
	laneClosureCnt->closureRegion = *closureReg;

	contentCnt->choice.laneClosureContainer = *laneClosureCnt;
	asn_sequence_add(&content->list.array, contentCnt);
	message->content = *content;

	xer_fprint(stdout, &asn_DEF_RoadSafetyMessage, message);

	// Encode RSM 
	tmx::messages::RsmEncodedMessage RsmEncodeMessage;
	auto _rsmMessage = new tmx::messages::RsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_rsmMessage->get_j2735_data());
	RsmEncodeMessage.set_data(TmxJ2735EncodedMessage<RoadSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
	free(message);
	free(frame_msg.get_j2735_data().get());
	std::cout << RsmEncodeMessage.get_payload_str() << std::endl;
	EXPECT_EQ(33,  RsmEncodeMessage.get_msgId());   
	std::string expectedRSMEncHex = "00215b0101051f41808022202000218181431f9fa15ac00000008000054edb8b1439665b0c194c0000281a40ae7e74902ba058a3fbb81b28fb720d1a3e7484448f9d1f960280024a9db7162872ccb618329820034012007401748a5900c8";
	EXPECT_EQ(expectedRSMEncHex, RsmEncodeMessage.get_payload_str());

	// Decode RSM
	auto rsm_ptr = RsmEncodeMessage.decode_j2735_message().get_j2735_data();
	EXPECT_EQ(12, rsm_ptr->commonContainer.eventInfo.eventUpdate);
}
// Test encoding of Road Safety Message with reduced speed zone content
TEST(RoadSafetyMessageTest, EncodeRoadSafetyMessageReduceSpeed) {

	/**
	* Populate RSM 
	*/      
	auto message = (RoadSafetyMessage_t*) calloc(1, sizeof(RoadSafetyMessage_t));
	auto commonContainer = (CommonContainer_t*) calloc(1, sizeof(CommonContainer_t));
	auto eventInfo = (EventInfo_t*) calloc(1, sizeof(EventInfo_t));

	// Event ID info
	auto eventID = (EventIdentifier_t*) calloc(1, sizeof(EventIdentifier_t));
	eventID->operatorID.present = RoadAuthorityID_PR_fullRdAuthID;

	// Road Authority OID
	uint32_t oid[] = {1,0,15628,4,1,17,1,0};
	int success = OBJECT_IDENTIFIER_set_arcs(&(eventID->operatorID.choice.fullRdAuthID), oid, sizeof(oid)/sizeof(oid[0]));
	if (success != 0) {
		std::cout << "Failed to set OID arcs" << std::endl;
		GTEST_FAIL();
	}
	uint8_t  my_bytes_uid[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
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

	// End Date time
	auto rsmEndDateTime = (DDateTime_t*) calloc(1, sizeof(DDateTime_t));
	auto endYear = (DYear_t*) calloc(1, sizeof(DYear_t));
	auto endMonth = (DMonth_t*) calloc(1, sizeof(DMonth_t));
	auto endDay = (DDay_t*) calloc(1, sizeof(DDay_t));
	auto endHour = (DHour_t*) calloc(1, sizeof(DHour_t));
	auto endMinute = (DMinute_t*) calloc(1, sizeof(DMinute_t));
	auto endSecond = (DSecond_t*) calloc(1, sizeof(DSecond_t));
	*endYear = 2024;
	*endMonth = 5;
	*endDay = 17;
	*endHour = 12;
	*endMinute = 0;
	*endSecond = 0;
	rsmEndDateTime->year = endYear;
	rsmEndDateTime->month = endMonth;
	rsmEndDateTime->day = endDay;
	rsmEndDateTime->hour = endHour;
	rsmEndDateTime->minute = endMinute;
	rsmEndDateTime->second = endSecond;
	eventInfo->endDateTime = rsmEndDateTime;

	// // Event recurrence list
	auto rsmEventRecurrence = (EventInfo::EventInfo__eventRecurrence*) calloc(1, sizeof(EventInfo::EventInfo__eventRecurrence));
	auto eventRecCnt = (EventRecurrence_t*) calloc(1, sizeof(EventRecurrence_t));
	eventRecCnt->monday = 1;
	eventRecCnt->tuesday = 1;
	eventRecCnt->wednesday = 1;
	eventRecCnt->thursday = 1;
	eventRecCnt->friday = 1;
	eventRecCnt->saturday = 1;
	eventRecCnt->sunday = 1;
	asn_sequence_add(&rsmEventRecurrence->list.array, eventRecCnt);
	eventInfo->eventRecurrence = rsmEventRecurrence;

	// ITIS cause codes
	eventInfo->causeCode = 2;
	auto subCode = (ITIS_ITIScodes_t*) calloc(1, sizeof(ITIS_ITIScodes_t));
	*subCode = 8026;
	eventInfo->subCauseCode = subCode;

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

	// Reduced Speed Zone Content
	contentCnt->present = ContentContainer_PR_rszContainer;

	auto reducedSpeedCnt = (ReducedSpeedZoneContainer_t*) calloc(1, sizeof(ReducedSpeedZoneContainer_t));
	reducedSpeedCnt->speedLimit.type = 3;
	reducedSpeedCnt->speedLimit.speed = 20;

	auto rszReg = (RegionInfo_t*) calloc(1, sizeof(RegionInfo_t));
	auto refPointRsz = (Position3D_t*) calloc(1, sizeof(Position3D_t));
	refPointRsz->lat = 423010836;
	refPointRsz->Long = -836990707;
	auto elevRsz = (Common_Elevation_t*) calloc(1, sizeof(Common_Elevation_t));
	*elevRsz = 2380;
	refPointRsz->elevation = elevRsz;
	rszReg->referencePoint = *refPointRsz;
	auto refPointType = (ReferencePointType_t*) calloc(1, sizeof(ReferencePointType_t));
	*refPointType = 0;
	rszReg->referencePointType = refPointType;

	auto rszApproachReg = (AreaType_t*) calloc(1, sizeof(AreaType_t));
	rszApproachReg->present = AreaType_PR_paths;
	auto pathList = (PathList_t*) calloc(1, sizeof(PathList_t));
	auto rszPath = (Path_t*) calloc(1, sizeof(Path_t));
	rszPath->pathWidth = 26;
	auto pathNode1 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode1->present = NodePointLLE_PR_node_3Doffset;
	pathNode1->choice.node_3Doffset.lat_offset = 14;
	pathNode1->choice.node_3Doffset.long_offset = 23;
	auto pathNode2 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode2->present = NodePointLLE_PR_node_3Doffset;
	pathNode2->choice.node_3Doffset.lat_offset = 1324;
	pathNode2->choice.node_3Doffset.long_offset = 50;
	auto pathNode3 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode3->present = NodePointLLE_PR_node_3Doffset;
	pathNode3->choice.node_3Doffset.lat_offset = 1945;
	pathNode3->choice.node_3Doffset.long_offset = 131;
	auto node3elevOff = (ElevOffset_t*) calloc(1, sizeof(ElevOffset_t));
	*node3elevOff = 10;
	pathNode3->choice.node_3Doffset.elev_offset = node3elevOff;
	auto pathNode4 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode4->present = NodePointLLE_PR_node_3Doffset;
	pathNode4->choice.node_3Doffset.lat_offset = 2645;
	pathNode4->choice.node_3Doffset.long_offset = 483;
	auto pathNode5 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode5->present = NodePointLLE_PR_node_3Doffset;
	pathNode5->choice.node_3Doffset.lat_offset = 3095;
	pathNode5->choice.node_3Doffset.long_offset = 956;
	auto pathNode6 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode6->present = NodePointLLE_PR_node_3Doffset;
	pathNode6->choice.node_3Doffset.lat_offset = 3475;
	pathNode6->choice.node_3Doffset.long_offset = 1538;
	auto pathNode7 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode7->present = NodePointLLE_PR_node_3Doffset;
	pathNode7->choice.node_3Doffset.lat_offset = 3685;
	pathNode7->choice.node_3Doffset.long_offset = 2024;
	asn_sequence_add(&rszPath->pathPoints.list.array, pathNode1);
	asn_sequence_add(&rszPath->pathPoints.list.array, pathNode2);
	asn_sequence_add(&rszPath->pathPoints.list.array, pathNode3);
	asn_sequence_add(&rszPath->pathPoints.list.array, pathNode4);
	asn_sequence_add(&rszPath->pathPoints.list.array, pathNode5);
	asn_sequence_add(&rszPath->pathPoints.list.array, pathNode6);
	asn_sequence_add(&rszPath->pathPoints.list.array, pathNode7);
	asn_sequence_add(&pathList->list.array, rszPath);
	rszApproachReg->choice.paths = *pathList;
	rszReg->approachRegion = rszApproachReg;
	reducedSpeedCnt->rszRegion = *rszReg;

	contentCnt->choice.rszContainer = *reducedSpeedCnt;
	asn_sequence_add(&content->list.array, contentCnt);
	message->content = *content;

	xer_fprint(stdout, &asn_DEF_RoadSafetyMessage, message);

	// Encode RSM 
	tmx::messages::RsmEncodedMessage RsmEncodeMessage;
	auto _rsmMessage = new tmx::messages::RsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_rsmMessage->get_j2735_data());
	RsmEncodeMessage.set_data(TmxJ2735EncodedMessage<RoadSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
	free(message);
	free(frame_msg.get_j2735_data().get());
	std::cout << RsmEncodeMessage.get_payload_str() << std::endl;
	EXPECT_EQ(33,  RsmEncodeMessage.get_msgId());   
	std::string expectedRSMEncHex = "00217d0f01051f41808022202000218181431f9fa15ac000003f3f42c5800000001fc087d68000054edb8b1439665b0c194c0000281a40ae7e74902ba058a3fbb81b28fb720d1a3e7484448f9d1f960018050953b6e2c50e5996c306530400682a400e802e914b20192c799810700a494ab078d260bc3bc49b2718092732c7e8";
	EXPECT_EQ(expectedRSMEncHex, RsmEncodeMessage.get_payload_str());

	// Decode RSM
	auto rsm_ptr = RsmEncodeMessage.decode_j2735_message().get_j2735_data();
	EXPECT_EQ(12, rsm_ptr->commonContainer.eventInfo.eventUpdate);

	
}
// Test encoding of Road Safety Message with reduced speed zone content
TEST(RoadSafetyMessageTest, EncodeRoadSafetyMessageDynamicInfo) {

	/**
	* Populate RSM 
	*/      
	auto message = (RoadSafetyMessage_t*) calloc(1, sizeof(RoadSafetyMessage_t));
	auto commonContainer = (CommonContainer_t*) calloc(1, sizeof(CommonContainer_t));
	auto eventInfo = (EventInfo_t*) calloc(1, sizeof(EventInfo_t));

	// Event ID info
	auto eventID = (EventIdentifier_t*) calloc(1, sizeof(EventIdentifier_t));
	eventID->operatorID.present = RoadAuthorityID_PR_fullRdAuthID;

	// Road Authority OID
	uint32_t oid[] = {1,0,15628,4,1,17,1,0};
	int success = OBJECT_IDENTIFIER_set_arcs(&(eventID->operatorID.choice.fullRdAuthID), oid, sizeof(oid)/sizeof(oid[0]));
	if (success != 0) {
		std::cout << "Failed to set OID arcs" << std::endl;
		GTEST_FAIL();
	}
	uint8_t  my_bytes_uid[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
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

	// End Date time
	auto rsmEndDateTime = (DDateTime_t*) calloc(1, sizeof(DDateTime_t));
	auto endYear = (DYear_t*) calloc(1, sizeof(DYear_t));
	auto endMonth = (DMonth_t*) calloc(1, sizeof(DMonth_t));
	auto endDay = (DDay_t*) calloc(1, sizeof(DDay_t));
	auto endHour = (DHour_t*) calloc(1, sizeof(DHour_t));
	auto endMinute = (DMinute_t*) calloc(1, sizeof(DMinute_t));
	auto endSecond = (DSecond_t*) calloc(1, sizeof(DSecond_t));
	*endYear = 2024;
	*endMonth = 5;
	*endDay = 17;
	*endHour = 12;
	*endMinute = 0;
	*endSecond = 0;
	rsmEndDateTime->year = endYear;
	rsmEndDateTime->month = endMonth;
	rsmEndDateTime->day = endDay;
	rsmEndDateTime->hour = endHour;
	rsmEndDateTime->minute = endMinute;
	rsmEndDateTime->second = endSecond;
	eventInfo->endDateTime = rsmEndDateTime;

	// Event recurrence list
	auto rsmEventRecurrence = (EventInfo::EventInfo__eventRecurrence*) calloc(1, sizeof(EventInfo::EventInfo__eventRecurrence));
	auto eventRecCnt = (EventRecurrence_t*) calloc(1, sizeof(EventRecurrence_t));
	eventRecCnt->monday = 1;
	eventRecCnt->tuesday = 1;
	eventRecCnt->wednesday = 1;
	eventRecCnt->thursday = 1;
	eventRecCnt->friday = 1;
	eventRecCnt->saturday = 1;
	eventRecCnt->sunday = 1;
	asn_sequence_add(&rsmEventRecurrence->list.array, eventRecCnt);
	eventInfo->eventRecurrence = rsmEventRecurrence;

	// ITIS cause codes
	eventInfo->causeCode = 2;
	auto subCode = (ITIS_ITIScodes_t*) calloc(1, sizeof(ITIS_ITIScodes_t));
	*subCode = 8026;
	eventInfo->subCauseCode = subCode;

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

	// Dynamic Content
	contentCnt->present = ContentContainer_PR_dynamicInfoContainer;

	auto dynamicInfoContainer = (DynamicInfoContainer_t*) calloc(1, sizeof(DynamicInfoContainer_t));
	dynamicInfoContainer->priority = 3;
	auto dmsString = (DynamicInfoContainer::DynamicInfoContainer__dmsSignString*) calloc(1, sizeof(DynamicInfoContainer::DynamicInfoContainer__dmsSignString));
	auto myString = (ShortString_t*) calloc(1, sizeof(ShortString_t));
	char* my_str = (char *) "Wrong Way Driver";
	uint8_t* my_bytes = reinterpret_cast<uint8_t *>(my_str);
	myString->buf = my_bytes;
	myString->size = strlen(my_str);
	asn_sequence_add(&dmsString->list.array, myString);
	dynamicInfoContainer->dmsSignString = *dmsString;
	dynamicInfoContainer->applicableRegion.referencePoint.lat = 389549610;
	dynamicInfoContainer->applicableRegion.referencePoint.Long = -771493030;
	auto appElev = (Common_Elevation_t*) calloc(1, sizeof(Common_Elevation_t));
	*appElev = 390;
	dynamicInfoContainer->applicableRegion.referencePoint.elevation = appElev;

	
	contentCnt->choice.dynamicInfoContainer = *dynamicInfoContainer;
	asn_sequence_add(&content->list.array, contentCnt);
	message->content = *content;

	xer_fprint(stdout, &asn_DEF_RoadSafetyMessage, message);

	// Encode RSM 
	tmx::messages::RsmEncodedMessage RsmEncodeMessage;
	auto _rsmMessage = new tmx::messages::RsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_rsmMessage->get_j2735_data());
	RsmEncodeMessage.set_data(TmxJ2735EncodedMessage<RoadSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
	free(message);
	free(frame_msg.get_j2735_data().get());
	std::cout << RsmEncodeMessage.get_payload_str() << std::endl;
	EXPECT_EQ(33,  RsmEncodeMessage.get_msgId());   
	std::string expectedRSMEncHex = "0021680f01051f41808022202000218181431f9fa15ac000003f3f42c5800000001fc087d68000054edb8b1439665b0c194c0000281a40ae7e74902ba058a3fbb81b28fb720d1a3e7484448f9d1f9606c1f5f96fdd9d057c3e5044e5a7b65e40299b9ee547a9b8ab2230c0";
	EXPECT_EQ(expectedRSMEncHex, RsmEncodeMessage.get_payload_str());

	// Decode RSM
	auto rsm_ptr = RsmEncodeMessage.decode_j2735_message().get_j2735_data();
	EXPECT_EQ(12, rsm_ptr->commonContainer.eventInfo.eventUpdate);
}
// Test encoding of Road Safety Message with incident content
TEST(RoadSafetyMessageTest, EncodeRoadSafetyMessageIncident) {

	/**
	* Populate RSM 
	*/      
	auto message = (RoadSafetyMessage_t*) calloc(1, sizeof(RoadSafetyMessage_t));
	auto commonContainer = (CommonContainer_t*) calloc(1, sizeof(CommonContainer_t));
	auto eventInfo = (EventInfo_t*) calloc(1, sizeof(EventInfo_t));

	// Event ID info
	auto eventID = (EventIdentifier_t*) calloc(1, sizeof(EventIdentifier_t));
	eventID->operatorID.present = RoadAuthorityID_PR_fullRdAuthID;

	// Road Authority OID
	uint32_t oid[] = {1,0,15628,4,1,17,1,0};
	int success = OBJECT_IDENTIFIER_set_arcs(&(eventID->operatorID.choice.fullRdAuthID), oid, sizeof(oid)/sizeof(oid[0]));
	if (success != 0) {
		std::cout << "Failed to set OID arcs" << std::endl;
		GTEST_FAIL();
	}
	uint8_t  my_bytes_uid[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
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

	

	

	// ITIS cause codes
	eventInfo->causeCode = 2;
	auto subCode = (ITIS_ITIScodes_t*) calloc(1, sizeof(ITIS_ITIScodes_t));
	*subCode = 8026;
	eventInfo->subCauseCode = subCode;

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

	// Incidents Content
	contentCnt->present = ContentContainer_PR_incidentsContainer;

	auto incidentsCnt = (IncidentsContainer_t*) calloc(1, sizeof(IncidentsContainer_t));
	auto responderType = (IncidentsContainer_t::IncidentsContainer__responderType*) calloc(1, sizeof(IncidentsContainer_t::IncidentsContainer__responderType));
	auto resp1 = (ITIS_ResponderGroupAffected_t*) calloc(1, sizeof(ITIS_ResponderGroupAffected_t));
	*resp1 = 9733;
	auto resp2 = (ITIS_ResponderGroupAffected_t*) calloc(1, sizeof(ITIS_ResponderGroupAffected_t));
	*resp2 = 9734;
	asn_sequence_add(&responderType->list.array, resp1);
	asn_sequence_add(&responderType->list.array, resp2);
	incidentsCnt->responderType = responderType;

	auto incRegion = (RegionInfo_t*) calloc(1, sizeof(RegionInfo_t));
	auto refPointInc = (Position3D_t*) calloc(1, sizeof(Position3D_t));
	refPointInc->lat = 423010836;
	refPointInc->Long = -836990707;
	auto elevInc = (Common_Elevation_t*) calloc(1, sizeof(Common_Elevation_t));
	*elevInc = 2380;
	refPointInc->elevation = elevInc;
	incRegion->referencePoint = *refPointInc;
	auto refPointType = (ReferencePointType_t*) calloc(1, sizeof(ReferencePointType_t));
	*refPointType = 0;
	incRegion->referencePointType = refPointType;

	auto incApproachReg = (AreaType_t*) calloc(1, sizeof(AreaType_t));
	incApproachReg->present = AreaType_PR_paths;
	auto pathList = (PathList_t*) calloc(1, sizeof(PathList_t));
	auto incPath = (Path_t*) calloc(1, sizeof(Path_t));
	incPath->pathWidth = 26;
	auto pathNode1 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode1->present = NodePointLLE_PR_node_3Doffset;
	pathNode1->choice.node_3Doffset.lat_offset = 14;
	pathNode1->choice.node_3Doffset.long_offset = 23;
	auto pathNode2 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode2->present = NodePointLLE_PR_node_3Doffset;
	pathNode2->choice.node_3Doffset.lat_offset = 1324;
	pathNode2->choice.node_3Doffset.long_offset = 50;
	asn_sequence_add(&incPath->pathPoints.list.array, pathNode1);
	asn_sequence_add(&incPath->pathPoints.list.array, pathNode2);
	asn_sequence_add(&pathList->list.array, incPath);
	incApproachReg->choice.paths = *pathList;
	incRegion->approachRegion = incApproachReg;
	incidentsCnt->incidentLocation = *incRegion;
	
	contentCnt->choice.incidentsContainer = *incidentsCnt;
	asn_sequence_add(&content->list.array, contentCnt);
	message->content = *content;

	xer_fprint(stdout, &asn_DEF_RoadSafetyMessage, message);

	// Encode RSM 
	tmx::messages::RsmEncodedMessage RsmEncodeMessage;
	auto _rsmMessage = new tmx::messages::RsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_rsmMessage->get_j2735_data());
	RsmEncodeMessage.set_data(TmxJ2735EncodedMessage<RoadSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
	free(message);
	free(frame_msg.get_j2735_data().get());
	std::cout << RsmEncodeMessage.get_payload_str() << std::endl;
	EXPECT_EQ(33,  RsmEncodeMessage.get_msgId());   
	std::string expectedRSMEncHex = "00215d0301051f41808022202000218181431f9fa15ac000000087d68000054edb8b1439665b0c194c0000281a40ae7e74902ba058a3fbb81b28fb720d1a3e7484448f9d1f96089214953b6e2c50e5996c3065304006802400e802e914b20190";
	EXPECT_EQ(expectedRSMEncHex, RsmEncodeMessage.get_payload_str());

	// Decode RSM
	auto rsm_ptr = RsmEncodeMessage.decode_j2735_message().get_j2735_data();
	EXPECT_EQ(12, rsm_ptr->commonContainer.eventInfo.eventUpdate);
}
// Test encoding of Road Safety Message with curve content
TEST(RoadSafetyMessageTest, EncodeRoadSafetyMessageCurve) {

	/**
	* Populate RSM 
	*/      
	auto message = (RoadSafetyMessage_t*) calloc(1, sizeof(RoadSafetyMessage_t));
	auto commonContainer = (CommonContainer_t*) calloc(1, sizeof(CommonContainer_t));
	auto eventInfo = (EventInfo_t*) calloc(1, sizeof(EventInfo_t));

	// Event ID info
	auto eventID = (EventIdentifier_t*) calloc(1, sizeof(EventIdentifier_t));
	eventID->operatorID.present = RoadAuthorityID_PR_fullRdAuthID;

	// Road Authority OID
	uint32_t oid[] = {1,0,15628,4,1,17,1,0};
	int success = OBJECT_IDENTIFIER_set_arcs(&(eventID->operatorID.choice.fullRdAuthID), oid, sizeof(oid)/sizeof(oid[0]));
	if (success != 0) {
		std::cout << "Failed to set OID arcs" << std::endl;
		GTEST_FAIL();
	}
	uint8_t  my_bytes_uid[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
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

	// ITIS cause codes
	eventInfo->causeCode = 2;
	auto subCode = (ITIS_ITIScodes_t*) calloc(1, sizeof(ITIS_ITIScodes_t));
	*subCode = 8026;
	eventInfo->subCauseCode = subCode;

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

	// Curve Content
	contentCnt->present = ContentContainer_PR_curveContainer;

	auto curveContainer = (CurveContainer_t*) calloc(1, sizeof(CurveContainer_t));
	curveContainer->advisorySpeed = 112;
	auto curveRegion = (RegionInfo_t*) calloc(1, sizeof(RegionInfo_t));
	auto refPointCurve = (Position3D_t*) calloc(1, sizeof(Position3D_t));
	refPointCurve->lat = 423010836;
	refPointCurve->Long = -836990707;
	auto elevCurve = (Common_Elevation_t*) calloc(1, sizeof(Common_Elevation_t));
	*elevCurve = 2380;
	refPointCurve->elevation = elevCurve;
	curveRegion->referencePoint = *refPointCurve;
	auto refPointType = (ReferencePointType_t*) calloc(1, sizeof(ReferencePointType_t));
	*refPointType = 0;
	curveRegion->referencePointType = refPointType;

	auto curveApproachReg = (AreaType_t*) calloc(1, sizeof(AreaType_t));
	curveApproachReg->present = AreaType_PR_paths;
	auto pathList = (PathList_t*) calloc(1, sizeof(PathList_t));
	auto curvePath = (Path_t*) calloc(1, sizeof(Path_t));
	curvePath->pathWidth = 26;
	auto pathNode1 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode1->present = NodePointLLE_PR_node_3Doffset;
	pathNode1->choice.node_3Doffset.lat_offset = 14;
	pathNode1->choice.node_3Doffset.long_offset = 23;
	auto pathNode2 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode2->present = NodePointLLE_PR_node_3Doffset;
	pathNode2->choice.node_3Doffset.lat_offset = 1324;
	pathNode2->choice.node_3Doffset.long_offset = 50;
	auto pathNode3 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode3->present = NodePointLLE_PR_node_3Doffset;
	pathNode3->choice.node_3Doffset.lat_offset = 1945;
	pathNode3->choice.node_3Doffset.long_offset = 131;
	auto node3elevOff = (ElevOffset_t*) calloc(1, sizeof(ElevOffset_t));
	*node3elevOff = 10;
	pathNode3->choice.node_3Doffset.elev_offset = node3elevOff;
	auto pathNode4 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode4->present = NodePointLLE_PR_node_3Doffset;
	pathNode4->choice.node_3Doffset.lat_offset = 2645;
	pathNode4->choice.node_3Doffset.long_offset = 483;
	auto pathNode5 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode5->present = NodePointLLE_PR_node_3Doffset;
	pathNode5->choice.node_3Doffset.lat_offset = 3095;
	pathNode5->choice.node_3Doffset.long_offset = 956;
	auto pathNode6 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode6->present = NodePointLLE_PR_node_3Doffset;
	pathNode6->choice.node_3Doffset.lat_offset = 3475;
	pathNode6->choice.node_3Doffset.long_offset = 1538;
	auto pathNode7 = (NodePointLLE_t*) calloc(1, sizeof(NodePointLLE_t));
	pathNode7->present = NodePointLLE_PR_node_3Doffset;
	pathNode7->choice.node_3Doffset.lat_offset = 3685;
	pathNode7->choice.node_3Doffset.long_offset = 2024;
	asn_sequence_add(&curvePath->pathPoints.list.array, pathNode1);
	asn_sequence_add(&curvePath->pathPoints.list.array, pathNode2);
	asn_sequence_add(&curvePath->pathPoints.list.array, pathNode3);
	asn_sequence_add(&curvePath->pathPoints.list.array, pathNode4);
	asn_sequence_add(&curvePath->pathPoints.list.array, pathNode5);
	asn_sequence_add(&curvePath->pathPoints.list.array, pathNode6);
	asn_sequence_add(&curvePath->pathPoints.list.array, pathNode7);
	asn_sequence_add(&pathList->list.array, curvePath);
	curveApproachReg->choice.paths = *pathList;
	curveRegion->approachRegion = curveApproachReg;
	curveContainer->curveRegion = curveRegion;

	contentCnt->choice.curveContainer = *curveContainer;
	asn_sequence_add(&content->list.array, contentCnt);
	message->content = *content;

	xer_fprint(stdout, &asn_DEF_RoadSafetyMessage, message);

	// Encode RSM 
	tmx::messages::RsmEncodedMessage RsmEncodeMessage;
	auto _rsmMessage = new tmx::messages::RsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_rsmMessage->get_j2735_data());
	RsmEncodeMessage.set_data(TmxJ2735EncodedMessage<RoadSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
	free(message);
	free(frame_msg.get_j2735_data().get());
	std::cout << RsmEncodeMessage.get_payload_str() << std::endl;
	EXPECT_EQ(33,  RsmEncodeMessage.get_msgId());   
	std::string expectedRSMEncHex = "0021740301051f41808022202000218181431f9fa15ac000000087d68000054edb8b1439665b0c194c0000281a40ae7e74902ba058a3fbb81b28fb720d1a3e7484448f9d1f960413812a76dc58a1cb32d860ca60800d054801d005d2296403258f33020e014929560f1a4c1787789364e30124e658fd00";
	EXPECT_EQ(expectedRSMEncHex, RsmEncodeMessage.get_payload_str());

	// Decode RSM
	auto rsm_ptr = RsmEncodeMessage.decode_j2735_message().get_j2735_data();
	EXPECT_EQ(12, rsm_ptr->commonContainer.eventInfo.eventUpdate);
}
// Test encoding of Road Safety Message with situation content
TEST(RoadSafetyMessageTest, EncodeRoadSafetyMessageSituation) {

	/**
	* Populate RSM 
	*/      
	auto message = (RoadSafetyMessage_t*) calloc(1, sizeof(RoadSafetyMessage_t));
	auto commonContainer = (CommonContainer_t*) calloc(1, sizeof(CommonContainer_t));
	auto eventInfo = (EventInfo_t*) calloc(1, sizeof(EventInfo_t));

	// Event ID info
	auto eventID = (EventIdentifier_t*) calloc(1, sizeof(EventIdentifier_t));
	eventID->operatorID.present = RoadAuthorityID_PR_fullRdAuthID;

	// Road Authority OID
	uint32_t oid[] = {1,0,15628,4,1,17,1,0};
	int success = OBJECT_IDENTIFIER_set_arcs(&(eventID->operatorID.choice.fullRdAuthID), oid, sizeof(oid)/sizeof(oid[0]));
	if (success != 0) {
		std::cout << "Failed to set OID arcs" << std::endl;
		GTEST_FAIL();
	}
	uint8_t  my_bytes_uid[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
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

	
	// ITIS cause codes
	eventInfo->causeCode = 2;
	auto subCode = (ITIS_ITIScodes_t*) calloc(1, sizeof(ITIS_ITIScodes_t));
	*subCode = 8026;
	eventInfo->subCauseCode = subCode;

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

	contentCnt->choice.situationalContainer = *situationalCnt;
	asn_sequence_add(&content->list.array, contentCnt);
	message->content = *content;

	xer_fprint(stdout, &asn_DEF_RoadSafetyMessage, message);

	// Encode RSM 
	tmx::messages::RsmEncodedMessage RsmEncodeMessage;
	auto _rsmMessage = new tmx::messages::RsmMessage(message);
	tmx::messages::MessageFrameMessage frame_msg(_rsmMessage->get_j2735_data());
	RsmEncodeMessage.set_data(TmxJ2735EncodedMessage<RoadSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));
	free(message);
	free(frame_msg.get_j2735_data().get());
	std::cout << RsmEncodeMessage.get_payload_str() << std::endl;
	EXPECT_EQ(33,  RsmEncodeMessage.get_msgId());   
	std::string expectedRSMEncHex = "0021680301051f41808022202000218181431f9fa15ac000000087d68000054edb8b1439665b0c194c0000281a40ae7e74902ba058a3fbb81b28fb720d1a3e7484448f9d1f960a854edb8b1439665b0c194c0214254edb8b1439665b0c194c1001a009003a00ba452c8064";
	EXPECT_EQ(expectedRSMEncHex, RsmEncodeMessage.get_payload_str());

	// Decode RSM
	auto rsm_ptr = RsmEncodeMessage.decode_j2735_message().get_j2735_data();
	EXPECT_EQ(12, rsm_ptr->commonContainer.eventInfo.eventUpdate);
}
// Test decoding of Road Safety Message
TEST(RoadSafetyMessageTest, DecodeRoadSafetyMessage) {
	// Decode any RSM
	std::string rsmInput = "0021430700e51f418080222020218181431f9fa0e6f7800b400fe0e0e0200000a66e7b951ea6e2ac88c306c1f5f96fdd9d057c3e5044e5a7b65e40299b9ee547a9b8ab2230c0";
	std::vector<uint8_t> byteArray = hexStringToByteArray(rsmInput);
	tmx::messages::RsmEncodedMessage decodedMessage;
	decodedMessage.set_data(byteArray);
	auto decodedRsmPtr = decodedMessage.decode_j2735_message().get_j2735_data();
	xer_fprint(stdout, &asn_DEF_RoadSafetyMessage, decodedRsmPtr.get());
}
#endif