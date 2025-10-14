#include <RSUHealthMonitorWorker.h>
#include <SNMPClient.h>
#include <gtest/gtest.h>

using namespace std;
using namespace tmx::utils;
using namespace tmx::utils::rsu::mib::rsu41;
using namespace tmx::utils::rsu::mib::ntcip1218;
using namespace tmx::messages;

namespace RSUHealthMonitor
{
    class test_RSUHealthMonitorWorker : public ::testing::Test
    {
    public:
        std::shared_ptr<RSUHealthMonitorWorker> _rsuWorker = std::make_shared<RSUHealthMonitorWorker>();
    };

    TEST_F(test_RSUHealthMonitorWorker, GetRSUStatusConfig)
    {
        RSUStatusConfigTable rsuStatusConfigTbl = _rsuWorker->GetRSUStatusConfig(tmx::utils::rsu::RSU_SPEC::RSU_4_1);
        EXPECT_EQ(14, rsuStatusConfigTbl.size());

    }

    TEST_F(test_RSUHealthMonitorWorker, validateAllRequiredFieldsPresent)
    {
        auto config = _rsuWorker->GetRSUStatusConfig(tmx::utils::rsu::RSU_SPEC::RSU_4_1);
        vector<string> requiredFields = {"rsuID", "rsuMibVersion", "rsuFirmwareVersion", "rsuManufacturer", "rsuGpsOutputString", "rsuMode", "rsuChanStatus"};
        EXPECT_TRUE(_rsuWorker->validateAllRequiredFieldsPresent(config, requiredFields));

        requiredFields = {"rsuID", "rsuMibVersion", "rsuFirmwareVersion"};
        EXPECT_FALSE(_rsuWorker->validateAllRequiredFieldsPresent(config, requiredFields));
    }

    TEST_F(test_RSUHealthMonitorWorker, ParseRSUGPS)
    {
        std::string gps_nmea_data = "$GPGGA,142440.00,3857.3065,N,07708.9734,W,2,18,0.65,86.18,M,-34.722,M,,*62";
        auto gps_map = _rsuWorker->ParseRSUGPS(gps_nmea_data);
        EXPECT_EQ(1, gps_map.size());
        double expected_latitude = 38.9551;
        double expected_longitude = -77.1496;
        for (auto itr = gps_map.begin(); itr != gps_map.end(); itr++)
        {
            EXPECT_NEAR(expected_latitude, itr->first, 0.001);
            EXPECT_NEAR(expected_longitude, itr->second, 0.001);
        }
        std::string invalid_gps_nmea_data = "$*GPGGA,invalid";
        auto gps_map_invalid = _rsuWorker->ParseRSUGPS(invalid_gps_nmea_data);
        EXPECT_EQ(0, gps_map_invalid.size());
    }

    TEST_F(test_RSUHealthMonitorWorker, getRSUStatus)
    {
        uint16_t port = 161;
        EXPECT_THROW(_rsuWorker->getRSUStatus(tmx::utils::rsu::RSU_SPEC::NTCIP_1218, "127.0.0.1", port, "testUser", "SHA-512", "testtesttest", "AES-256", "test1234", "authPriv", 1000), std::runtime_error);

        EXPECT_THROW(_rsuWorker->getRSUStatus(tmx::utils::rsu::RSU_SPEC::NTCIP_1218, "127.0.0.1", port, "testUser", "SHA-512", "test1234", "AES-256", "test1234", "authPriv", 1000), std::runtime_error);

        EXPECT_THROW( _rsuWorker->getRSUStatus(tmx::utils::rsu::RSU_SPEC::NTCIP_1218, "127.0.0.1", port, "testUser", "SHA-512", "test1234", "AES-256", "test1234", "authPriv", 1000), std::runtime_error);

    }

    TEST_F(test_RSUHealthMonitorWorker, convertJsonToTMXMsg)
    {
        Json::Value json;
        json["rsuID"] = "RSU4.1";
        json["rsuMode"] = 4;
        auto rsuStatusTmxMsg = _rsuWorker->convertJsonToTMXMsg(json);
        string expectedStr = "{\"rsuID\":\"RSU4.1\",\"rsuMode\":4}\n";
        EXPECT_EQ(expectedStr, rsuStatusTmxMsg.to_string());
    }

    TEST_F(test_RSUHealthMonitorWorker, populateJson)
    {
        Json::Value rsuStatusJson;
        snmp_response_obj stringObj;
        stringObj.type = snmp_response_obj::response_type::STRING;
        std::string gps_nmea_data = "$GPGGA,142440.00,3857.3065,N,07708.9734,W,2,18,0.65,86.18,M,-34.722,M,,*62";
        vector<char> rgps_nmea_data_c;
        copy(gps_nmea_data.begin(), gps_nmea_data.end(), back_inserter(rgps_nmea_data_c));
        stringObj.val_string = rgps_nmea_data_c;

        auto json = _rsuWorker->populateJson("rsuGpsOutputString", stringObj);
        double expected_latitude = 38.9551;
        double expected_longitude = -77.1496;
        EXPECT_NEAR(expected_latitude, json["rsuGpsOutputStringLatitude"].asDouble(), 0.001);
        EXPECT_NEAR(expected_longitude, json["rsuGpsOutputStringLongitude"].asDouble(), 0.001);
        for(const auto& key: json.getMemberNames())
        {
            rsuStatusJson[key] = json[key];
        }

        snmp_response_obj intObj;
        intObj.type = snmp_response_obj::response_type::INTEGER;
        intObj.val_int = 4;

        json = _rsuWorker->populateJson("rsuMode", intObj);
        EXPECT_EQ(4, json["rsuMode"].asInt64());
        rsuStatusJson["rsuMode"] = json["rsuMode"];

        Json::FastWriter fasterWirter;
        string json_str = fasterWirter.write(rsuStatusJson);
        string expectedStr = "{\"rsuGpsOutputString\":\"$GPGGA,142440.00,3857.3065,N,07708.9734,W,2,18,0.65,86.18,M,-34.722,M,,*62\",\"rsuGpsOutputStringLatitude\":38.955108330000002,\"rsuGpsOutputStringLongitude\":-77.149556669999996,\"rsuMode\":4}\n";
        EXPECT_EQ(expectedStr, json_str);
        EXPECT_EQ(4, _rsuWorker->getJsonKeys(rsuStatusJson).size());
        EXPECT_EQ(1, _rsuWorker->getJsonKeys(json).size());
    }

}