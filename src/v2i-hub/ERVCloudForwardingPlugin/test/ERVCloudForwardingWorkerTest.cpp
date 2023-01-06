
#include <gtest/gtest.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "ERVCloudForwardingWorker.h"

using namespace std;
namespace unit_test
{
   class ERVCloudForwardingWorkerTest : public ::testing::Test
   {
   public:
      BsmMessage *_bsmMessage;
      BsmMessage *_bsmMessagePartII;
      ERVCloudForwardingWorkerTest(){};
      ~ERVCloudForwardingWorkerTest(){};

      void SetUp()
      {
         /***
          * Construct BSM
          * ***/
         BasicSafetyMessage_t *message = (BasicSafetyMessage_t *)calloc(1, sizeof(BasicSafetyMessage_t));
         // Populate BSMcoreData
         char *my_str = (char *)"sender_id";
         uint8_t *my_bytes = reinterpret_cast<uint8_t *>(my_str);
         message->coreData.msgCnt = 1;
         uint8_t my_bytes_id[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
         message->coreData.id.buf = my_bytes_id;
         message->coreData.id.size = sizeof(my_bytes_id);
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
         uint8_t my_bytes_brakes[1] = {8};
         message->coreData.brakes.wheelBrakes.buf = my_bytes_brakes;
         message->coreData.brakes.wheelBrakes.size = sizeof(my_bytes_brakes);
         message->coreData.brakes.wheelBrakes.bits_unused = 3;

         // vehicle size
         message->coreData.size.length = 500;
         message->coreData.size.width = 300;
         _bsmMessage = new BsmMessage(message);

         /**
          * Construct BSM with PartII
          * */
         BasicSafetyMessage_t *messagePartII = (BasicSafetyMessage_t *)calloc(1, sizeof(BasicSafetyMessage_t));
         // Populate BSMcoreData
         messagePartII->coreData.msgCnt = 1;
         messagePartII->coreData.id.buf = my_bytes_id;
         messagePartII->coreData.id.size = sizeof(my_bytes_id);
         messagePartII->coreData.secMark = 1023;
         messagePartII->coreData.lat = 38954961;
         messagePartII->coreData.Long = -77149303;
         messagePartII->coreData.elev = 72;
         messagePartII->coreData.speed = 100;
         messagePartII->coreData.heading = 12;
         messagePartII->coreData.angle = 10;
         messagePartII->coreData.transmission = 0;

         // position accuracy
         messagePartII->coreData.accuracy.orientation = 100;
         messagePartII->coreData.accuracy.semiMajor = 200;
         messagePartII->coreData.accuracy.semiMinor = 200;

         // Acceleration set
         messagePartII->coreData.accelSet.lat = 100;
         messagePartII->coreData.accelSet.Long = 300;
         messagePartII->coreData.accelSet.vert = 100;
         messagePartII->coreData.accelSet.yaw = 0;

         // populate brakes
         messagePartII->coreData.brakes.abs = 1;
         messagePartII->coreData.brakes.scs = 1;
         messagePartII->coreData.brakes.traction = 1;
         messagePartII->coreData.brakes.brakeBoost = 1;
         messagePartII->coreData.brakes.auxBrakes = 1;
         messagePartII->coreData.brakes.wheelBrakes.buf = my_bytes_brakes;
         messagePartII->coreData.brakes.wheelBrakes.size = sizeof(my_bytes_brakes);
         messagePartII->coreData.brakes.wheelBrakes.bits_unused = 3;

         // vehicle size
         messagePartII->coreData.size.length = 500;
         messagePartII->coreData.size.width = 300;

         // BSM BSMpartIIExtension
         auto bsmPartII = (BasicSafetyMessage::BasicSafetyMessage__partII *)calloc(1, sizeof(BasicSafetyMessage::BasicSafetyMessage__partII));
         auto partIICnt = (BSMpartIIExtension_t *)calloc(1, sizeof(BSMpartIIExtension_t));
         partIICnt->partII_Id = 1;
         partIICnt->partII_Value.present = BSMpartIIExtension__partII_Value_PR_SpecialVehicleExtensions;
         auto specialVEx = (SpecialVehicleExtensions_t *)calloc(1, sizeof(SpecialVehicleExtensions_t));
         auto emergencyDetails = (EmergencyDetails_t *)calloc(1, sizeof(EmergencyDetails_t));
         emergencyDetails->lightsUse = LightbarInUse_inUse;
         auto resp_type = (ResponseType_t *)calloc(1, sizeof(ResponseType_t));
         *resp_type = ResponseType_emergency;
         emergencyDetails->responseType = resp_type;
         emergencyDetails->sirenUse = SirenInUse_inUse;
         specialVEx->vehicleAlerts = emergencyDetails;
         partIICnt->partII_Value.choice.SpecialVehicleExtensions = *specialVEx;
         asn_sequence_add(&bsmPartII->list.array, partIICnt);
         messagePartII->partII = bsmPartII;

         // BSM regional extension
         auto regional = (BasicSafetyMessage::BasicSafetyMessage__regional *)calloc(1, sizeof(BasicSafetyMessage::BasicSafetyMessage__regional));
         auto reg_bsm = (Reg_BasicSafetyMessage *)calloc(1, sizeof(Reg_BasicSafetyMessage));
         reg_bsm->regionId = 128;
         reg_bsm->regExtValue.present = Reg_BasicSafetyMessage__regExtValue_PR_BasicSafetyMessage_addGrpCarma;
         auto carma_bsm_data = (BasicSafetyMessage_addGrpCarma_t *)calloc(1, sizeof(BasicSafetyMessage_addGrpCarma_t));
         auto carma_bsm_destination_points = (BasicSafetyMessage_addGrpCarma::BasicSafetyMessage_addGrpCarma__routeDestinationPoints *)calloc(1, sizeof(BasicSafetyMessage_addGrpCarma::BasicSafetyMessage_addGrpCarma__routeDestinationPoints));
         auto point = (Position3D_t *)calloc(1, sizeof(Position3D_t));
         auto dummy_lat = 12;
         auto dummy_long = 1312;
         point->lat = dummy_lat;
         point->Long = dummy_long;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point);
         auto point2 = (Position3D_t *)calloc(1, sizeof(Position3D_t));
         point2->lat = dummy_lat + 1000;
         point2->Long = dummy_long + 1000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point2);
         auto point3 = (Position3D_t *)calloc(1, sizeof(Position3D_t));
         point3->lat = dummy_lat + 2000;
         point3->Long = dummy_long + 2000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point3);
         auto point4 = (Position3D_t *)calloc(1, sizeof(Position3D_t));
         point4->lat = dummy_lat + 3000;
         point4->Long = dummy_long + 3000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point4);
         auto point5 = (Position3D_t *)calloc(1, sizeof(Position3D_t));
         point5->lat = dummy_lat + 4000;
         point5->Long = dummy_long + 4000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point5);
         auto point6 = (Position3D_t *)calloc(1, sizeof(Position3D_t));
         point6->lat = dummy_lat + 5000;
         point6->Long = dummy_long + 5000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point6);
         auto point7 = (Position3D_t *)calloc(1, sizeof(Position3D_t));
         point7->lat = dummy_lat + 6000;
         point7->Long = dummy_long + 6000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point7);
         auto point8 = (Position3D_t *)calloc(1, sizeof(Position3D_t));
         point8->lat = dummy_lat + 7000;
         point8->Long = dummy_long + 7000;
         asn_sequence_add(&carma_bsm_destination_points->list.array, point8);
         carma_bsm_data->routeDestinationPoints = carma_bsm_destination_points;
         reg_bsm->regExtValue.choice.BasicSafetyMessage_addGrpCarma = *carma_bsm_data;
         asn_sequence_add(&regional->list.array, reg_bsm);
         messagePartII->regional = regional;
         _bsmMessagePartII = new BsmMessage(messagePartII);
      }
      void TearDown()
      {
      }
   };

   TEST_F(ERVCloudForwardingWorkerTest, encodeBSMHex)
   {
      // BSM without partII
      string bsmHex = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::encodeBSMHex(*_bsmMessage);
      string expectedBSMHex = "001425004043030280ffdbfba868b3584ec40824646400320032000c888fc834e37fff0aaa960fa0";
      ASSERT_EQ(expectedBSMHex, bsmHex);

      // ERV (Emergency Response Vehicle) BSM with partII
      bsmHex = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::encodeBSMHex(*_bsmMessagePartII);
      expectedBSMHex = "00146e604043030280ffdbfba868b3584ec40824646400320032000c888fc834e37fff0aaa960fa0040d082408804278d693a431ad275c7c6b49d9e8d693b60e35a4f0dc6b49deef1ad27a6235a4f16b8d693e2b1ad279afc6b49f928d693d54e35a5007c6b49ee8f1ad2823235a4f93b8";
      ASSERT_EQ(expectedBSMHex, bsmHex);
   }

   TEST_F(ERVCloudForwardingWorkerTest, constructERVBSMRequest)
   {
      // BSM without partII
      string bsmReq = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::constructERVBSMRequest(*_bsmMessage);
      ASSERT_EQ("", bsmReq);

      // ERV BSM with partII
      bsmReq = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::constructERVBSMRequest(*_bsmMessagePartII);
      string expectedBSMHex = "00146e604043030280ffdbfba868b3584ec40824646400320032000c888fc834e37fff0aaa960fa0040d082408804278d693a431ad275c7c6b49d9e8d693b60e35a4f0dc6b49deef1ad27a6235a4f16b8d693e2b1ad279afc6b49f928d693d54e35a5007c6b49ee8f1ad2823235a4f93b8";
      string expectedBSMReq = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><BSMRequest><id>" + expectedBSMHex + "</id><route><point><latitude>12</latitude><longitude>1312</longitude></point><point><latitude>1012</latitude><longitude>2312</longitude></point><point><latitude>2012</latitude><longitude>3312</longitude></point><point><latitude>3012</latitude><longitude>4312</longitude></point><point><latitude>4012</latitude><longitude>5312</longitude></point><point><latitude>5012</latitude><longitude>6312</longitude></point><point><latitude>6012</latitude><longitude>7312</longitude></point><point><latitude>7012</latitude><longitude>8312</longitude></point><route></BSMRequest>";
      ASSERT_EQ(expectedBSMReq, bsmReq);
   }

   TEST_F(ERVCloudForwardingWorkerTest, IsBSMFromERV)
   {
      // BSM without partII
      auto result = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::IsBSMFromERV(*_bsmMessage);
      ASSERT_FALSE(result);

      // ERV BSM with partII
      result = ERVCloudForwardingPlugin::ERVCloudForwardingWorker::IsBSMFromERV(*_bsmMessagePartII);
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
      std::string expected_xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><RSULocationRequest><id>" + rsu_identifier + "</id><latitude>3895510833<latitude><longitude>-7714955667</longitude><v2xhubPort>" + std::to_string(v2xhubPort) + "</v2xhubPort></RSULocationRequest>";
      ASSERT_EQ(expected_xml, xml_str);
   }
} // namespace unit_test
