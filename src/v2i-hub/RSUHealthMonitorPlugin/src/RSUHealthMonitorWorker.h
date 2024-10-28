#pragma once
#include <string>
#include "RSU_MIB_4_1.h"
#include "NTCIP_1218_MIB.h"
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
#include "RSUConfigurationList.h"

using namespace std;
using namespace tmx::utils;
using namespace tmx::utils::rsu41::mib::oid;
using namespace tmx::utils::ntcip1218::mib::oid;
using namespace tmx::messages;

namespace RSUHealthMonitor
{

    struct RSUFieldOIDStruct
    {
        string field;
        string oid;
        bool required; // Indicate whether this field is required to before broadcasting the RSU status.
    };

    /**
     * RSUStatusTable is custom defined RSU status information.
     * The fields are a subset of the fields from the RSU MIB definition used to quantify the health of the RSU.
     */
    using RSUStatusConfigTable = vector<RSUFieldOIDStruct>;

    class RSUHealthMonitorWorker
    {
    private:
        // A map of RSU MIB version used and RSUStatusTable
        shared_ptr<map<RSUMibVersion, RSUStatusConfigTable>> _RSUSTATUSConfigMapPtr;

        /**
         * @brief Populate the RSU status table with the specified version of OIDs and fields.
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
         * @param configTbl RSU Status configration table to compare with.
         * @param fields Input RSU fields to verify
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
         * @param mibVersion The RSU MIB version used.
         * @param _rsuIp RSU IP address.
         * @param _snmpPort SNMP port.
         * @param _securityUser Security user used for SNMP authentication.
         * @param _authProtocol The authentication protocol (MD5|SHA|SHA-224|SHA-256|SHA-384|SHA-512).
         * @param _authPassPhrase The authentication protocol pass phrase.
         * @param _privProtocol The privacy protocol (DES|AES|AES-192|AES-256).
         * @param _privPassPhrase The privacy protocol pass phrase.
         * @param _securityLevel security level: authPriv or authNoPriv.
         * @param timeout Session time out.
         */
        Json::Value getRSUStatus(const RSUMibVersion &mibVersion, const string &_rsuIp, uint16_t &_snmpPort, const string &_securityUser, const std::string &_authProtocol, const std::string &_authPassPhrase, const std::string &_privProtocol, const std::string &_privPassPhrase,const string &_securityLevel, long timeout);

        /***
         *@brief Convert the JSON message into TMX message
         @param json Input Json value
         @return RSUStatusMessage TMX message
        */
        RSUStatusMessage convertJsonToTMXMsg(const Json::Value &json) const;

        /**
         * @brief Populate Json with snmp response object.
         * @param field The field that maps to an OID.
         * @param response The response returned by SNMP call for the OID.
         * @return Json value populated with response object.
         */
        Json::Value populateJson(const string &field, const snmp_response_obj &response) const;

        /**
         * @brief List the keys from the input Json values
         * @param json Input JSON values
         * @return vector of key strings
        */
        vector<string> getJsonKeys(const Json::Value &json) const;

        // Delete move constructor
        RSUHealthMonitorWorker(RSUHealthMonitorWorker &&worker) = delete;

        // Delete copy constructor
        RSUHealthMonitorWorker(RSUHealthMonitorWorker &worker) = delete;
    };
} // namespace RSUHealthMonitor
