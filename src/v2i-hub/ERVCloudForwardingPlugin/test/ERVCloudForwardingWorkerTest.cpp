
#include <gtest/gtest.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include "ERVCloudForwardingWorker.h"

using namespace std;
namespace unit_test
{
   class ERVCloudForwardingWorkerTest : public ::testing::Test
   {
   public:
      BsmMessage _bsmMessage;
      BsmMessage _bsmMessagePartII;
      ERVCloudForwardingWorkerTest() = default;
      ~ERVCloudForwardingWorkerTest() = default;

      void SetUp() override
      {
         /***
          * Construct BSM
          * ***/
         auto message = tmx::messages::j2735::j2735_create<tmx::messages::BsmTraits>();
         // Populate BSMcoreData
         message->coreData.msgCnt = 1; 
         {
            char my_bytes_id[4] = {(char)1, (char)12, (char)12, (char)10};
            bool failed = OCTET_STRING_fromBuf(&message->coreData.id, my_bytes_id, sizeof(my_bytes_id));
            // If operation fails, unit test is no longer valid
            ASSERT_EQ(failed, 0);
         }
         message->coreData.secMark = 1023;
         message->coreData.lat = 38954961;
         message->coreData.Long = -77149303;
         message->coreData.elev = 72;
         message->coreData.speed = 100;
         message->coreData.heading = 12;
         message->coreData.angle = 10;
         message->coreData.transmission = 0;

         // position accuracy
         message->coreData.accuracy.orientation = 100;
         message->coreData.accuracy.semiMajor = 200;
         message->coreData.accuracy.semiMinor = 200;

         // Acceleration set
         message->coreData.accelSet.lat = 100;
         message->coreData.accelSet.Long = 300;
         message->coreData.accelSet.vert = 100;
         message->coreData.accelSet.yaw = 0;

         // populate brakes
         message->coreData.brakes.abs = 1;
         message->coreData.brakes.scs = 1;
         message->coreData.brakes.traction = 1;
         message->coreData.brakes.brakeBoost = 1;
         message->coreData.brakes.auxBrakes = 1;
         {
            // Allocate memory for wheelBrakes BIT_STRING. Otherwise Asan gives error :
            // ERROR: AddressSanitizer: attempting free on address which was not malloc()-ed
            uint8_t  *my_bytes_brakes = static_cast<uint8_t *>(calloc(1, sizeof(uint8_t)));
            *my_bytes_brakes = 8;
            message->coreData.brakes.wheelBrakes.buf = my_bytes_brakes; // allow 0,1,2,3,4
         }
         message->coreData.brakes.wheelBrakes.size =1; // allow 0,1,2,3,4	
         message->coreData.brakes.wheelBrakes.bits_unused = 3; // allow 0,1,2,3,4	

         // vehicle size
         message->coreData.size.length = 500;
         message->coreData.size.width = 300;
         _bsmMessage =  BsmMessage(message);

         /**
          * Construct BSM with PartII
          * */
         auto message2 = tmx::messages::j2735::j2735_create<tmx::messages::BsmTraits>();
         // Populate BSMcoreData
         message2->coreData.msgCnt = 1;
         {
            char my_bytes_id[4] = {(char)1, (char)12, (char)12, (char)10};
            bool failed = OCTET_STRING_fromBuf(&message2->coreData.id, my_bytes_id, sizeof(my_bytes_id));
            // If operation fails, unit test is no longer valid
            ASSERT_EQ(failed, 0);
         }
         message2->coreData.secMark = 1023;
         message2->coreData.lat = 38954961;
         message2->coreData.Long = -77149303;
         message2->coreData.elev = 72;
         message2->coreData.speed = 100;
         message2->coreData.heading = 12;
         message2->coreData.angle = 10;
         message2->coreData.transmission = 0;

         // position accuracy
         message2->coreData.accuracy.orientation = 100;
         message2->coreData.accuracy.semiMajor = 200;
         message2->coreData.accuracy.semiMinor = 200;

         // Acceleration set
         message2->coreData.accelSet.lat = 100;
         message2->coreData.accelSet.Long = 300;
         message2->coreData.accelSet.vert = 100;
         message2->coreData.accelSet.yaw = 0;

         // populate brakes
         message2->coreData.brakes.abs = 1;
         message2->coreData.brakes.scs = 1;
         message2->coreData.brakes.traction = 1;
         message2->coreData.brakes.brakeBoost = 1;
         message2->coreData.brakes.auxBrakes = 1;
         {
            // Allocate memory for wheelBrakes BIT_STRING. Otherwise Asan gives error :
            // ERROR: AddressSanitizer: attempting free on address which was not malloc()-ed
            uint8_t  *my_bytes_brakes = static_cast<uint8_t *>(calloc(1, sizeof(uint8_t)));
            *my_bytes_brakes = 8;
            message2->coreData.brakes.wheelBrakes.buf = my_bytes_brakes; // allow 0,1,2,3,4
         }
         message2->coreData.brakes.wheelBrakes.size =1;
         message2->coreData.brakes.wheelBrakes.bits_unused = 3;

         // vehicle size
         message2->coreData.size.length = 500;
         message2->coreData.size.width = 300;

         // BSM BSMpartIIExtension
         auto bsmPartII = tmx::messages::j2735::AllocAsn<BasicSafetyMessage::BasicSafetyMessage__partII>();
         auto partIICnt = tmx::messages::j2735::AllocAsn<BSMpartIIExtension_t>();
         partIICnt->partII_Id = 1;
         partIICnt->partII_Value.present = BSMpartIIExtension__partII_Value_PR_SpecialVehicleExtensions;
         SpecialVehicleExtensions_t specialVEx;
         // Avoid uninitialized pointers
         memset(&specialVEx, 0, sizeof(specialVEx));
         auto emergencyDetails = tmx::messages::j2735::AllocAsn<EmergencyDetails_t>();
         emergencyDetails->lightsUse = LightbarInUse_inUse;
         auto resp_type = tmx::messages::j2735::AllocAsn<ResponseType_t>();
         *resp_type = ResponseType_emergency;
         emergencyDetails->responseType = resp_type;
         emergencyDetails->sirenUse = SirenInUse_inUse;
         specialVEx.vehicleAlerts = emergencyDetails;
         partIICnt->partII_Value.choice.SpecialVehicleExtensions = specialVEx;
         asn_sequence_add(&bsmPartII->list.array, partIICnt);
         message2->partII = bsmPartII;
         // BSM regional extension
         auto regional = tmx::messages::j2735::AllocAsn<BasicSafetyMessage::BasicSafetyMessage__regional>();
         auto reg_bsm = tmx::messages::j2735::AllocAsn<Reg_BasicSafetyMessage>();
         reg_bsm->regionId = 128;
         reg_bsm->regExtValue.present = Reg_BasicSafetyMessage__regExtValue_PR_BasicSafetyMessage_addGrpCarma;
         BasicSafetyMessage_addGrpCarma_t carma_bsm_data;
         // Avoid uninitialized pointers
         memset(&carma_bsm_data, 0, sizeof(carma_bsm_data));
         auto carma_bsm_destination_points = tmx::messages::j2735::AllocAsn<BasicSafetyMessage_addGrpCarma::BasicSafetyMessage_addGrpCarma__routeDestinationPoints>();
         auto point = tmx::messages::j2735::AllocAsn<Position3D_t>();
         auto dummy_lat = 12;
         auto dummy_long = 1312;
         point->lat = dummy_lat;
         point->Long = dummy_long;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point);
         auto point2 = tmx::messages::j2735::AllocAsn<Position3D_t>();
         point2->lat = dummy_lat + 1000;
         point2->Long = dummy_long + 1000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point2);
         auto point3 = tmx::messages::j2735::AllocAsn<Position3D_t>();
         point3->lat = dummy_lat + 2000;
         point3->Long = dummy_long + 2000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point3);
         auto point4 = tmx::messages::j2735::AllocAsn<Position3D_t>();
         point4->lat = dummy_lat + 3000;
         point4->Long = dummy_long + 3000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point4);
         auto point5 = tmx::messages::j2735::AllocAsn<Position3D_t>();
         point5->lat = dummy_lat + 4000;
         point5->Long = dummy_long + 4000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point5);
         auto point6 = tmx::messages::j2735::AllocAsn<Position3D_t>();
         point6->lat = dummy_lat + 5000;
         point6->Long = dummy_long + 5000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point6);
         auto point7 = tmx::messages::j2735::AllocAsn<Position3D_t>();
         point7->lat = dummy_lat + 6000;
         point7->Long = dummy_long + 6000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point7);
         auto point8 = tmx::messages::j2735::AllocAsn<Position3D_t>();
         point8->lat = dummy_lat + 7000;
         point8->Long = dummy_long + 7000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point8);
         carma_bsm_data.routeDestinationPoints = carma_bsm_destination_points;
         reg_bsm->regExtValue.choice.BasicSafetyMessage_addGrpCarma = carma_bsm_data;
         asn_sequence_add(&regional->list.array, reg_bsm);
         message2->regional = regional;
         _bsmMessagePartII = BsmMessage(message2);
      }
   };

   TEST_F(ERVCloudForwardingWorkerTest, encodeBSMHex)
   {
      // BSM without partII
      string bsmHex = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::encodeBSMHex(_bsmMessage);
      string expectedBSMHex = "001425004043030280ffdbfba868b3584ec40824646400320032000c888fc834e37fff0aaa960fa0";
      ASSERT_EQ(expectedBSMHex, bsmHex);

      // ERV (Emergency Response Vehicle) BSM with partII
      bsmHex = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::encodeBSMHex(_bsmMessagePartII);
      expectedBSMHex = "00146e604043030280ffdbfba868b3584ec40824646400320032000c888fc834e37fff0aaa960fa0040d082408804278d693a431ad275c7c6b49d9e8d693b60e35a4f0dc6b49deef1ad27a6235a4f16b8d693e2b1ad279afc6b49f928d693d54e35a5007c6b49ee8f1ad2823235a4f93b8";
      ASSERT_EQ(expectedBSMHex, bsmHex);
   }

   TEST_F(ERVCloudForwardingWorkerTest, constructERVBSMRequest)
   {
      uint16_t v2xhubPort = 11111;
      // BSM without partII
      string bsmReq = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::constructERVBSMRequest(_bsmMessage, v2xhubPort);
      ASSERT_EQ("", bsmReq);

      // ERV BSM with partII
      bsmReq = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::constructERVBSMRequest(_bsmMessagePartII, v2xhubPort);
      string expectedBSMHex = "00146e604043030280ffdbfba868b3584ec40824646400320032000c888fc834e37fff0aaa960fa0040d082408804278d693a431ad275c7c6b49d9e8d693b60e35a4f0dc6b49deef1ad27a6235a4f16b8d693e2b1ad279afc6b49f928d693d54e35a5007c6b49ee8f1ad2823235a4f93b8";
      string expectedBSMReq = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><BSMRequest><id>" + expectedBSMHex + "</id><v2xhubPort>"+ std::to_string(v2xhubPort)+"</v2xhubPort><route><point><latitude>38954961</latitude><longitude>-77149303</longitude></point><point><latitude>12</latitude><longitude>1312</longitude></point><point><latitude>1012</latitude><longitude>2312</longitude></point><point><latitude>2012</latitude><longitude>3312</longitude></point><point><latitude>3012</latitude><longitude>4312</longitude></point><point><latitude>4012</latitude><longitude>5312</longitude></point><point><latitude>5012</latitude><longitude>6312</longitude></point><point><latitude>6012</latitude><longitude>7312</longitude></point><point><latitude>7012</latitude><longitude>8312</longitude></point></route></BSMRequest>";
      ASSERT_EQ(expectedBSMReq, bsmReq);
   }

   TEST_F(ERVCloudForwardingWorkerTest, IsBSMFromERV)
   {
      // BSM without partII
      auto result = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::IsBSMFromERV(_bsmMessage);
      ASSERT_FALSE(result);

      // ERV BSM with partII
      result = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::IsBSMFromERV(_bsmMessagePartII);
      ASSERT_TRUE(result);
   }

   TEST_F(ERVCloudForwardingWorkerTest, ParseGPS)
   {
      std::string gps_nmea_data = "$GPGGA,142440.00,3857.3065,N,07708.9734,W,2,18,0.65,86.18,M,-34.722,M,,*62";
      auto gps_map = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::ParseGPS(gps_nmea_data);
      ASSERT_EQ(1, gps_map.size());
      long expected_latitude = 3895510833;
      long expected_longitude = -7714955667;
      for (auto itr = gps_map.begin(); itr != gps_map.end(); itr++)
      {
         ASSERT_EQ(expected_latitude, itr->first);
         ASSERT_EQ(expected_longitude, itr->second);
      }
      std::string invalid_gps_nmea_data = "$*GPGGA,invalid";
      auto gps_map_invalid = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::ParseGPS(invalid_gps_nmea_data);
      ASSERT_EQ(0, gps_map_invalid.size());
   }

   TEST_F(ERVCloudForwardingWorkerTest, constructRSULocationRequest)
   {
      std::string rsuName = "west_intersection_rsu";
      auto uuid = boost::uuids::random_generator()();
      string rsu_identifier = rsuName + "_" + boost::lexical_cast<std::string>(uuid);
      long latitude = 3895510833;
      long longitude = -7714955667;
      uint16_t v2xhubPort = 44444;
      auto xml_str = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::constructRSULocationRequest(rsu_identifier, v2xhubPort, latitude, longitude);
      std::string expected_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><RSULocationRequest><id>" + rsu_identifier + "</id><latitude>3895510833</latitude><longitude>-7714955667</longitude><v2xhubPort>" + std::to_string(v2xhubPort) + "</v2xhubPort></RSULocationRequest>";
      ASSERT_EQ(expected_xml, xml_str);
   }
} // namespace unit_test
