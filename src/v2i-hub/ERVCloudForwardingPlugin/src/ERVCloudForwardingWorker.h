
#ifndef ERVCLOUDFORWARDINGWORKER_H_
#define ERVCLOUDFORWARDINGWORKER_H_

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <tmx/tmx.h>
#include <tmx/messages/IvpJ2735.h>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <nmeaparse/nmea.h>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace tmx;
using namespace tmx::messages;

namespace ERVCloudForwardingPlugin
{
    class ERVCloudForwardingWorker
    {
    public:
        ERVCloudForwardingWorker() = delete;
        ~ERVCloudForwardingWorker() = delete;
        /**
         * @brief Encode BSM object and return an encoded hex string
         *
         * @param msg The BSM object to encode
         * @return Hex string
         */
        static string encodeBSMHex(BsmMessage &msg);
        /**
         * @brief Create a BSM request if the BSM is sent from an ERV(Emergency Response Vehicle)
         * @param msg The BSM object
         * @return The BSM request in XML format
         */
        static string constructERVBSMRequest(BsmMessage &msg);
        /**
         * @brief Check whether the BSM is sent from an ERV
         * @param msg The BSM object
         * @return true if the BSM is from ERV
         * @return false if the BSM is NOT from ERV
         */
        static bool IsBSMFromERV(BsmMessage &msg);
        /**
         * @brief Parse NMEA GPS sentense and return GPS related data
         * @param gps_nmea_data NMEA GPS sentense
         * @return map<long, long>  A map of latitude and longitude
         */
        static map<long, long> ParseGPS(std::string &gps_nmea_data);
        /**
         * @brief Construct a registering RSU location request to the cloud
         *
         * @param rsu_identifier A unique identifier to identify an RSU
         * @param latitude RSU GeoLocation latitude
         * @param longitude  RSU GeoLocation longitude
         * @param v2xhub_web_port v2xhub server port to receive BSM from cloud
         * @return string XML format request
         */
        static string constructRSULocationRequest(std::string &rsu_identifier, uint16_t v2xhub_web_port, long latitude, long longitude);
    };
}
#endif