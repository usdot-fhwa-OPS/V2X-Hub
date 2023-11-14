#pragma once
#include <string>
#include "RSU_MIB_4_1.h"
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <nmeaparse/nmea.h>
#include "PluginLog.h"
#include <jsoncpp/json/json.h>
#include "SNMPClient.h"
#include <boost/algorithm/string/predicate.hpp>
#include "RSUStatusMessage.h"

using namespace std;
using namespace tmx::utils;
using namespace tmx::utils::rsu41::mib::oid;
using namespace tmx::messages;

namespace RSUHealthMonitor
{
    enum class RSUMibVersion
    {
        UNKOWN_MIB_V = 0,
        RSUMIB_V_4_1 = 1,
        RSUMIB_V_1218 = 2
    };

    struct RSUFieldOIDStruct
    {
        string field;
        string oid;
        bool required; // Indicate whether this field is required to before broadcasting the RSU status.
    };

    /**
     * RSUStatusTable is custom defined RSU status information.
     * The fields are a subset of the fields from the RSU MIB definition used to quantify the health of the RSU. https://github.com/certificationoperatingcouncil/COC_TestSpecs/blob/master/AppNotes/RSU/RSU-MIB.txt
     */
    using RSUStatusConfigTable = vector<RSUFieldOIDStruct>;

    class RSUHealthMonitorWorker
    {
    private:
        // A map of RSU MIB version used and RSUStatusTable
        shared_ptr<map<RSUMibVersion, RSUStatusConfigTable>> _RSUSTATUSConfigMapPtr;

        /**
         * @brief Poupate the RSU status table with the specified version of OIDs and fields.
         * Private: Only allow to initialze the RSU STATUS MAP once
         * @param mibVersion specified
         * @return RSUStatusTable the self defined RSU status https://usdot-carma.atlassian.net/wiki/spaces/WFD2/pages/2640740360/RSU+Health+Monitor+Plugin+Design
         */
        RSUStatusConfigTable constructRsuStatusConfigTable(const RSUMibVersion &mibVersion) const;

    public:
        // Populate the RSU Status Table with predefined fields and their mapping OIDs in constructor
        RSUHealthMonitorWorker();

        // Access to the RSU status table based in the RSU MIB version provided
        RSUStatusConfigTable GetRSUStatusConfig(const RSUMibVersion &mibVersion) const;

        /**
         * @brief determine if all required fields in the RSU config map _RSUSTATUSConfigMapPtr present in the input fields
         * Use _RSUSTATUSConfigMapPtr RSU status config map that defines all fields and whether the fields are required.
         * @param RSUStatusConfigTable RSU Status configration table to compare with.
         * @param vector<string> Input RSU fields to verify
         * @return True if all required fields found. Otherwise, false.
         */
        bool validateAllRequiredFieldsPresent(const RSUHealthMonitor::RSUStatusConfigTable &configTbl, const vector<string> &fields) const;

        /**
         * @brief Parse NMEA GPS sentense and return GPS related data
         * @param gps_nmea_data NMEA GPS sentense
         * @return map<double, double>  A map of latitude and longitude
         */
        std::map<double, double> ParseRSUGPS(const std::string &gps_nmea_data) const;

        /**
         * @brief Sending SNMP V3 requests to get info for each field in the RSUStatusConfigTable, and return the RSU status in JSON
         * Use RSU Status configuration table include RSU field, OIDs, and whether fields  are required or optional
         * @param RSUMibVersion The RSU MIB version used
         * @param string RSU IP address
         * @param uint16_t SNMP port
         * @param string security user used for SNMP authentication
         * @param string authentication password
         * @param string security level: authPriv or authNoPriv.
         * @param long session time out
         */
        Json::Value getRSUStatus(const RSUMibVersion &mibVersion, const string &_rsuIp, uint16_t &_snmpPort, const string &_securityUser, const string &_authPassPhrase, const string &_securityLevel, long timeout);

        /***
         *@brief Convert the JSON message into TMX message
         @param Json Input Json value
         @return RSUStatusMessage TMX message
        */
        RSUStatusMessage convertJsonToTMXMsg(const Json::Value &json) const;

        /**
         * @brief Populate Json with snmp response object.
         * @param string The field that maps to an OID.
         * @param snmp_response_obj The response returned by SNMP call for the OID.
         * @return Json value populated with response object.
         */
        Json::Value populateJson(const string &field, const snmp_response_obj &response) const;

        // Delete move constructor
        RSUHealthMonitorWorker(RSUHealthMonitorWorker &&worker) = delete;

        // Delete copy constructor
        RSUHealthMonitorWorker(RSUHealthMonitorWorker &worker) = delete;
    };
} // namespace RSUHealthMonitor
