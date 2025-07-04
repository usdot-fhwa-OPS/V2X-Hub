
/**
 * WARNING: DO NOT EDIT THIS FILE
 *
 * This source file was generated from the CPlusPlusGenerator.xsl stylesheet
 *
 * Your changes will be overwritten when this tool executes again.  In order
 * to edit the contents, you must go into the source XML files
 *
 * @author Greg Baumgardner
 */
 

#ifndef INCLUDE_TMXAPIMESSAGES_H_
#define INCLUDE_TMXAPIMESSAGES_H_

#ifndef CONSTEXPR
#if __cplusplus > 199711L
#define CONSTEXPR constexpr
#else
#ifdef __GNUC__
#define CONSTEXPR __attribute__ ((unused))
#else
#define CONSTEXPR
#endif
#endif
#endif

		
	
namespace tmx {
		
namespace messages {
			
namespace api {
				
			
enum MsgType 
{
		Error = 0,
		Register = 1,
		Subscribe = 2,
		Status = 3,
		Config = 4,
		EventLog = 5
};
		
		
static CONSTEXPR const char *MSGTYPE_ERROR_STRING = "__error";
static CONSTEXPR const char *MSGTYPE_REGISTER_STRING = "__register";
static CONSTEXPR const char *MSGTYPE_SUBSCRIBE_STRING = "__subscribe";
static CONSTEXPR const char *MSGTYPE_STATUS_STRING = "__status";
static CONSTEXPR const char *MSGTYPE_CONFIG_STRING = "__config";
static CONSTEXPR const char *MSGTYPE_EVENTLOG_STRING = "__eventLog";
			
enum MsgSubType 
{
		None = 0,
		J2735 = 1,
		basicSafetyMessage_D = 2,
		basicSafetyMessageVerbose_D = 3,
		commonSafetyRequest_D = 4,
		emergencyVehicleAlert_D = 5,
		intersectionCollision_D = 6,
		mapData_D = 7,
		nmeaCorrections_D = 8,
		probeDataManagement_D = 9,
		probeVehicleData_D = 10,
		roadSideAlert_D = 11,
		rtcmCorrections_D = 12,
		signalPhaseAndTimingMessage_D = 13,
		signalRequestMessage_D = 14,
		signalStatusMessage_D = 15,
		travelerInformation_D = 16,
		uperFrame_D = 17,
		mapData = 18,
		signalPhaseAndTimingMessage = 19,
		basicSafetyMessage = 20,
		commonSafetyRequest = 21,
		emergencyVehicleAlert = 22,
		intersectionCollision = 23,
		nmeaCorrections = 24,
		probeDataManagement = 25,
		probeVehicleData = 26,
		roadSideAlert = 27,
		rtcmCorrections = 28,
		signalRequestMessage = 29,
		signalStatusMessage = 30,
		travelerInformation = 31,
		personalSafetyMessage = 32,
		roadSafetyMessage = 33,
		roadWeatherMessage = 34,
		probeDataConfigMessage = 35,
		probeDataReportMessage = 36,
		tollAdvertisementMessage = 37,
		tollUsageMessage = 38,
		tollUsageAckMessage = 39,
		cooperativeControlMessage = 40,
		sensorDataSharingMessage = 41,
		maneuverSharingAndCoordinatingMessage = 42,
		roadGeometryAndAttributes = 43,
		personalSafetyMessage2 = 44,
		trafficSignalPhaseAndTiming = 45,
		signalControlAndPrioritizationRequest = 46,
		signalControlAndPrioritizationStatus = 47,
		roadUserChargingConfigMessage = 48,
		roadUserChargingReportMessage = 49,
		trafficLightStatusMessage = 50,
		// Start Test Messages at end of range to allow for new messages added to be 
		// in ascending order
		testMessage00 = 240,
		testMessage01 = 241,
		testMessage02 = 242,
		testMessage03 = 243,
		testMessage04 = 244,
		testMessage05 = 245,
		testMessage06 = 246,
		testMessage07 = 247,
		testMessage08 = 248,
		testMessage09 = 249,
		testMessage10 = 250,
		testMessage11 = 251,
		testMessage12 = 252,
		testMessage13 = 253,
		testMessage14 = 254,
		testMessage15 = 255,
		J2735_end = 256,
		GID = 300
};
		
		
static CONSTEXPR const char *MSGSUBTYPE_NONE_STRING = "None";
static CONSTEXPR const char *MSGSUBTYPE_J2735_STRING = "J2735";
static CONSTEXPR const char *MSGSUBTYPE_BASICSAFETYMESSAGE_D_STRING = "BSM";
static CONSTEXPR const char *MSGSUBTYPE_BASICSAFETYMESSAGEVERBOSE_D_STRING = "BSMV";
static CONSTEXPR const char *MSGSUBTYPE_COMMONSAFETYREQUEST_D_STRING = "CSR";
static CONSTEXPR const char *MSGSUBTYPE_EMERGENCYVEHICLEALERT_D_STRING = "EVA";
static CONSTEXPR const char *MSGSUBTYPE_INTERSECTIONCOLLISION_D_STRING = "IC";
static CONSTEXPR const char *MSGSUBTYPE_MAPDATA_D_STRING = "MAP";
static CONSTEXPR const char *MSGSUBTYPE_NMEACORRECTIONS_D_STRING = "NMEA";
static CONSTEXPR const char *MSGSUBTYPE_PROBEDATAMANAGEMENT_D_STRING = "PDM";
static CONSTEXPR const char *MSGSUBTYPE_PROBEVEHICLEDATA_D_STRING = "PVD";
static CONSTEXPR const char *MSGSUBTYPE_ROADSIDEALERT_D_STRING = "RSA";
static CONSTEXPR const char *MSGSUBTYPE_RTCMCORRECTIONS_D_STRING = "RTCM";
static CONSTEXPR const char *MSGSUBTYPE_SIGNALPHASEANDTIMINGMESSAGE_D_STRING = "SPAT";
static CONSTEXPR const char *MSGSUBTYPE_SIGNALREQUESTMESSAGE_D_STRING = "SRM";
static CONSTEXPR const char *MSGSUBTYPE_SIGNALSTATUSMESSAGE_D_STRING = "SSM";
static CONSTEXPR const char *MSGSUBTYPE_TRAVELERINFORMATION_D_STRING = "TIM";
static CONSTEXPR const char *MSGSUBTYPE_UPERFRAME_D_STRING = "UPERframe";
static CONSTEXPR const char *MSGSUBTYPE_MAPDATA_STRING = "MAP-P";
static CONSTEXPR const char *MSGSUBTYPE_SIGNALPHASEANDTIMINGMESSAGE_STRING = "SPAT-P";
static CONSTEXPR const char *MSGSUBTYPE_BASICSAFETYMESSAGE_STRING = "BSM";
static CONSTEXPR const char *MSGSUBTYPE_COMMONSAFETYREQUEST_STRING = "CSR";
static CONSTEXPR const char *MSGSUBTYPE_EMERGENCYVEHICLEALERT_STRING = "EVA";
static CONSTEXPR const char *MSGSUBTYPE_INTERSECTIONCOLLISION_STRING = "IC";
static CONSTEXPR const char *MSGSUBTYPE_NMEACORRECTIONS_STRING = "NMEA";
static CONSTEXPR const char *MSGSUBTYPE_PROBEDATAMANAGEMENT_STRING = "PDM";
static CONSTEXPR const char *MSGSUBTYPE_PROBEVEHICLEDATA_STRING = "PVD";
static CONSTEXPR const char *MSGSUBTYPE_ROADSIDEALERT_STRING = "RSA";
static CONSTEXPR const char *MSGSUBTYPE_RTCMCORRECTIONS_STRING = "RTCM";
static CONSTEXPR const char *MSGSUBTYPE_SIGNALREQUESTMESSAGE_STRING = "SRM";
static CONSTEXPR const char *MSGSUBTYPE_SIGNALSTATUSMESSAGE_STRING = "SSM";
static CONSTEXPR const char *MSGSUBTYPE_TRAVELERINFORMATION_STRING = "TIM";
static CONSTEXPR const char *MSGSUBTYPE_PERSONALSAFETYMESSAGE_D_STRING = "PSM";
static CONSTEXPR const char *MSGSUBTYPE_PERSONALSAFETYMESSAGE_STRING = "PSM-P";
static CONSTEXPR const char *MSGSUBTYPE_PERSONALMOBILITYMESSAGE_STRING = "PMM";
static CONSTEXPR const char *MSGSUBTYPE_ROADSAFETYMESSAGE_STRING = "RSM";
static CONSTEXPR const char *MSGSUBTYPE_ROADWEATHERMESSAGE_STRING = "RWM";
static CONSTEXPR const char *MSGSUBTYPE_PROBEDATACONFIGMESSAGE_STRING = "PDC";
static CONSTEXPR const char *MSGSUBTYPE_PROBEDATAREPORTMESSAGE_STRING = "PDR";
static CONSTEXPR const char *MSGSUBTYPE_TOLLADVERTISEMENTMESSAGE_STRING = "TAM";
static CONSTEXPR const char *MSGSUBTYPE_TOLLUSAGEMESSAGE_STRING = "TUM";
static CONSTEXPR const char *MSGSUBTYPE_TOLLUSAGEACKMESSAGE_STRING = "TUMack";
static CONSTEXPR const char *MSGSUBTYPE_COOPERATIVECONTROLMESSAGE_STRING = "CCM";
static CONSTEXPR const char *MSGSUBTYPE_SENSORDATASHARINGMESSAGE_STRING = "SDSM";
static CONSTEXPR const char *MSGSUBTYPE_MANEUVERSHARINGANDCOORDINATINGMESSAGE_STRING = "MSCM";
static CONSTEXPR const char *MSGSUBTYPE_ROADGEOMETRYANDATTRIBUTES_STRING = "RGA";
static CONSTEXPR const char *MSGSUBTYPE_PERSONALSAFETYMESSAGE2_STRING = "PSM2";
static CONSTEXPR const char *MSGSUBTYPE_TRAFFICSIGNALPHASEANDTIMING_STRING = "TSPaT";
static CONSTEXPR const char *MSGSUBTYPE_SIGNALCONTROLANDPRIORITIZATIONREQUEST_STRING = "SCPR";
static CONSTEXPR const char *MSGSUBTYPE_SIGNALCONTROLANDPRIORITIZATIONSTATUS_STRING = "SCPS";
static CONSTEXPR const char *MSGSUBTYPE_ROADUSERCHARGINGCONFIGMESSAGE_STRING = "RUCCM";
static CONSTEXPR const char *MSGSUBTYPE_ROADUSERCHARGINGREPORTMESSAGE_STRING = "RUCRM";
static CONSTEXPR const char *MSGSUBTYPE_TRAFFICLIGHTSTATUSMESSAGE_STRING = "TLSM";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE00_STRING = "TMSG00-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE01_STRING = "TMSG01-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE02_STRING = "TMSG02-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE03_STRING = "TMSG03-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE04_STRING = "TMSG04-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE05_STRING = "TMSG05-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE06_STRING = "TMSG06-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE07_STRING = "TMSG07-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE08_STRING = "TMSG08-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE09_STRING = "TMSG09-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE10_STRING = "TMSG10-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE11_STRING = "TMSG11-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE12_STRING = "TMSG12-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE13_STRING = "TMSG13-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE14_STRING = "TMSG14-P";
static CONSTEXPR const char *MSGSUBTYPE_TESTMESSAGE15_STRING = "TMSG15-P";
static CONSTEXPR const char *MSGSUBTYPE_J2735_END_STRING = "J2735_end";
static CONSTEXPR const char *MSGSUBTYPE_GID_STRING = "GID";
			
enum Encoding 
{
		String = 0,
		JSON = 1,
		XML = 2,
		ByteArray = 3,
		ASN1_BER = 4,
		ASN1_UPER = 5
};
		
		
static CONSTEXPR const char *ENCODING_STRING_STRING = "string";
static CONSTEXPR const char *ENCODING_JSON_STRING = "json";
static CONSTEXPR const char *ENCODING_XML_STRING = "xmlstring";
static CONSTEXPR const char *ENCODING_BYTEARRAY_STRING = "bytearray/hexstring";
static CONSTEXPR const char *ENCODING_ASN1_BER_STRING = "asn.1-ber/hexstring";
static CONSTEXPR const char *ENCODING_ASN1_UPER_STRING = "asn.1-uper/hexstring";
			
enum Status 
{
		Unknown = 0,
		Started = 1,
		Running = 2,
		Stale = 3,
		Stopped = 4
};
		
		
static CONSTEXPR const char *STATUS_UNKNOWN_STRING = "Unknown";
static CONSTEXPR const char *STATUS_STARTED_STRING = "Started, waiting for connection...";
static CONSTEXPR const char *STATUS_RUNNING_STRING = "Running";
static CONSTEXPR const char *STATUS_STALE_STRING = "Connection going stale";
static CONSTEXPR const char *STATUS_STOPPED_STRING = "Stopped / Disconnected";

enum msgPSID
{
	None_PSID = 0x00,
	mapData_PSID = 0x8002,
	signalPhaseAndTimingMessage_PSID = 0x8002,
	basicSafetyMessage_PSID = 0x20,
	commonSafetyRequest_PSID = 0x20,
	emergencyVehicleAlert_PSID = 0x8005,
	intersectionCollision_PSID = 0x8002,
	nmeaCorrections_PSID = 0x8000,
	probeDataManagement_PSID = 0x8004,
	probeVehicleData_PSID = 0x8004,
	roadSideAlert_PSID = 0x8003,
	rtcmCorrections_PSID = 0x8000,
	signalRequestMessage_PSID = 0xE0000016,
	signalStatusMessage_PSID = 0x8002,
	travelerInformation_PSID = 0x8003,
	personalSafetyMessage_PSID = 0x27,
	roadSafetyMessage_PSID = 0x8003,
	roadWeatherMessage_PSID = 0x204099,
	probeDataConfigMessage_PSID = 0x8004,
	probeDataReportMessage_PSID = 0x8004,
	tollAdvertisementMessage_PSID = 0x800F,
	tollUsageMessage_PSID = 0x800F,
	tollUsageAckMessage_PSID = 0x800F,
	cooperativeControlMessage_PSID = 0x800E,
	sensorDataSharingMessage_PSID = 0x8010,
	maneuverSharingAndCoordinatingMessage_PSID = 0x8011,
	roadGeometryAndAttributes_PSID = 0xE0000017,
	personalSafetyMessage2_PSID = 0x27,
	trafficSignalPhaseAndTiming_PSID = 0x8002,
	signalControlAndPrioritizationRequest_PSID = 0xE0000016,
	signalControlAndPrioritizationStatus_PSID = 0xE0000015,
	roadUserChargingConfigMessage_PSID = 0x800F,
	roadUserChargingReportMessage_PSID = 0x800F,
	trafficLightStatusMessage_PSID = 0x8002,
	testMessage00_PSID = 0xBFEE,
	testMessage01_PSID = 0xBFEE,
	testMessage02_PSID = 0xBFEE,
	testMessage03_PSID = 0xBFEE,
	testMessage04_PSID = 0x8003,
	testMessage05_PSID = 0x8003,
	testMessage06_PSID = 0xBFEE,
	testMessage07_PSID = 0xBFEE,
	testMessage08_PSID = 0xBFEE,
	testMessage09_PSID = 0xBFEE,
	testMessage10_PSID = 0xBFEE,
	testMessage11_PSID = 0xBFEE,
	testMessage12_PSID = 0xBFEE,
	testMessage13_PSID = 0xBFEE,
	testMessage14_PSID = 0xBFEE,
	testMessage15_PSID = 0xBFEE
};

static CONSTEXPR const char *MSGPSID_NONE_PSID_STRING = "None";
static CONSTEXPR const char *MSGPSID_MAPDATA_PSID_STRING = "0x8002";
static CONSTEXPR const char *MSGPSID_SIGNALPHASEANDTIMINGMESSAGE_PSID_STRING = "0x8002";
static CONSTEXPR const char *MSGPSID_BASICSAFETYMESSAGE_PSID_STRING = "0x20";
static CONSTEXPR const char *MSGPSID_COMMONSAFETYREQUEST_PSID_STRING = "0x20";
static CONSTEXPR const char *MSGPSID_EMERGENCYVEHICLEALERT_PSID_STRING = "0x8005";
static CONSTEXPR const char *MSGPSID_INTERSECTIONCOLLISION_PSID_STRING = "0x8002";
static CONSTEXPR const char *MSGPSID_NMEACORRECTIONS_PSID_STRING = "0x8000";
static CONSTEXPR const char *MSGPSID_PROBEDATAMANAGEMENT_PSID_STRING = "0x8004";
static CONSTEXPR const char *MSGPSID_PROBEVEHICLEDATA_PSID_STRING = "0x8004";
static CONSTEXPR const char *MSGPSID_ROADSIDEALERT_PSID_STRING = "0x8003";
static CONSTEXPR const char *MSGPSID_RTCMCORRECTIONS_PSID_STRING = "0x8000";
static CONSTEXPR const char *MSGPSID_SIGNALREQUESTMESSAGE_PSID_STRING = "0xE0000016";
static CONSTEXPR const char *MSGPSID_SIGNALSTATUSMESSAGE_PSID_STRING = "0x8002";
static CONSTEXPR const char *MSGPSID_TRAVELERINFORMATION_PSID_STRING = "0x8003";
static CONSTEXPR const char *MSGPSID_PERSONALSAFETYMESSAGE_PSID_STRING = "0x27";
static CONSTEXPR const char *MSGPSID_ROADSAFETYMESSAGE_PSID_STRING = "0x8003";
static CONSTEXPR const char *MSGPSID_ROADWEATHERMESSAGE_PSID_STRING = "0x204099";
static CONSTEXPR const char *MSGPSID_PROBEDATACONFIGMESSAGE_PSID_STRING = "0x8004";
static CONSTEXPR const char *MSGPSID_PROBEDATAREPORTMESSAGE_PSID_STRING = "0x8004";
static CONSTEXPR const char *MSGPSID_TOLLADVERTISEMENTMESSAGE_PSID_STRING = "0x800F";
static CONSTEXPR const char *MSGPSID_TOLLUSAGEMESSAGE_PSID_STRING = "0x800F";
static CONSTEXPR const char *MSGPSID_TOLLUSAGEACKMESSAGE_PSID_STRING = "0x800F";
static CONSTEXPR const char *MSGPSID_COOPERATIVECONTROLMESSAGE_PSID_STRING = "0x800E";
static CONSTEXPR const char *MSGPSID_SENSORDATASHARINGMESSAGE_PSID_STRING = "0x8010";
static CONSTEXPR const char *MSGPSID_MANEUVERSHARINGANDCOORDINATINGMESSAGE_PSID_STRING = "0x8011";
static CONSTEXPR const char *MSGPSID_ROADGEOMETRYANDATTRIBUTES_PSID_STRING = "0xE0000017";
static CONSTEXPR const char *MSGPSID_PERSONALSAFETYMESSAGE2_PSID_STRING = "0x27";
static CONSTEXPR const char *MSGPSID_TRAFFICSIGNALPHASEANDTIMING_PSID_STRING = "0x8002";
static CONSTEXPR const char *MSGPSID_SIGNALCONTROLANDPRIORITIZATIONREQUEST_PSID_STRING = "0xE0000016";
static CONSTEXPR const char *MSGPSID_SIGNALCONTROLANDPRIORITIZATIONSTATUS_PSID_STRING = "0xE0000015";
static CONSTEXPR const char *MSGPSID_ROADUSERCHARGINGCONFIGMESSAGE_PSID_STRING = "0x800F";
static CONSTEXPR const char *MSGPSID_ROADUSERCHARGINGREPORTMESSAGE_PSID_STRING = "0x800F";
static CONSTEXPR const char *MSGPSID_TRAFFICLIGHTSTATUSMESSAGE_PSID_STRING = "0x8002";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE00_PSID_STRING = "0xBFEE";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE01_PSID_STRING = "0xBFEE";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE02_PSID_STRING = "0xBFEE";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE03_PSID_STRING = "0xBFEE";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE04_PSID_STRING = "0x8003";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE05_PSID_STRING = "0x8003";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE06_PSID_STRING = "0xBFEE";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE07_PSID_STRING = "0xBFEE";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE08_PSID_STRING = "0xBFEE";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE09_PSID_STRING = "0xBFEE";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE10_PSID_STRING = "0xBFEE";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE11_PSID_STRING = "0xBFEE";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE12_PSID_STRING = "0xBFEE";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE13_PSID_STRING = "0xBFEE";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE14_PSID_STRING = "0xBFEE";
static CONSTEXPR const char *MSGPSID_TESTMESSAGE15_PSID_STRING = "0xBFEE";

} /* End namespace api */
		
} /* End namespace messages */
	
} /* End namespace tmx */

		
#endif /* INCLUDE_TMXAPIMESSAGES_H_ */
	
