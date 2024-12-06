#pragma once

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <vector>
#include "PluginLog.h"

#include "SNMPClientException.h"

namespace tmx::utils
{

    enum class request_type
    {
        GET,
        SET,
        OTHER // Processing this request type is not a defined behavior, included for testing only
    };

    /** @brief A struct to hold the value being sent to the TSC, can be integer or string. Type needs to be defined*/
    struct snmp_response_obj
    {
        /** @brief The type of value being requested or set, on the TSC */
        enum class response_type
        {
            INTEGER,
            STRING
        };

        // snmp response values can be any asn.1 supported types.
        // Integer and string values can be processed here
        int64_t val_int = 0;
        std::vector<char> val_string;
        response_type type;

        inline bool operator==(const snmp_response_obj &obj2) const
        {
            return val_int == obj2.val_int && val_string == obj2.val_string && type == obj2.type;
        }
    };

    class snmp_client
    {
    private:
        /*variables to store an snmp session*/
        // struct that holds information about who we're going to be talking to
        // We need to declare 2 of these, one to fill info with and second which is
        // a pointer returned by the library
        struct snmp_session session;
        struct snmp_session *ss;

        /*Structure to hold all of the information that we're going to send to the remote host*/
        struct snmp_pdu *pdu;
        /*Structure to hold response from the remote host*/
        struct snmp_pdu *response;

        /*OID is going to hold the location of the information which we want to receive. It will need a size as well*/
        oid OID[MAX_OID_LEN];
        size_t OID_len = MAX_OID_LEN;

        // Declare missing OID len definitions
        #define USM_PRIV_PROTO_AES192_LEN 9
        #define USM_PRIV_PROTO_AES256_LEN 9
        #define USM_PRIV_PROTO_AES192_CISCO_LEN 11
        #define USM_PRIV_PROTO_AES256_CISCO_LEN 11

        // Values from config
        /*Target device IP address*/
        std::string ip_;
        /*Target device NTCIP port*/
        int port_ = 0;
        /*Target community for establishing snmp communication*/
        std::string community_ = "public";
        /* net-snmp version definition: SNMP_VERSION_1:0 SNMP_VERSION_2c:1 SNMP_VERSION_2u:2 SNMP_VERSION_3:3
        https://github.com/net-snmp/net-snmp/blob/master/include/net-snmp/library/snmp.h */
        int snmp_version_ = SNMP_VERSION_3; // default to 3 since previous versions not compatable currently
        /*Time after which the the snmp request times out*/
        int timeout_ = 10000;
        /**
         * @brief Helper method for populating snmp_respons_obj with SNMP get response.
         * @param val response object
         * @param response pdu
         */
        void process_snmp_get_response(snmp_response_obj &val,  const snmp_pdu &response) const;
        /**
         * @brief Helper method for logging successful SNMP set responses
         * @param val response object
         * @param input_oid OID
         */
        void process_snmp_set_response( const snmp_response_obj &val,  const std::string &input_oid) const;

    public:
        /** @brief Overloaded constructor for Traffic Signal Controller Service client.
         *  Uses the arguments provided to establish an snmpv1 connection.
         * @param ip The ip, as a string, to establish an snmp communication with an snmp server.
         * @param port Target port as integer on the host for snmp communication.
         * @param community The community id as a string. Defaults to "public" if unassigned.
         * @param snmp_user Security user used for SNMP authentication.
         * @param securityLevel Security level: authPriv or authNoPriv.
         * @param authPassPhrase The authentication protocol pass phrase.
         * @param snmp_version The snmp_version as defined in net-snmp. Default to 0 if unassigned.
         *                     net-snmp version definition: SNMP_VERSION_1:0 SNMP_VERSION_2c:1 SNMP_VERSION_2u:2 SNMP_VERSION_3:3"
         * @param timeout The time in microseconds after which an snmp session request expires. Defaults to 100 if unassigned
         * **/
        snmp_client(const std::string &ip, const int &port, const std::string &community, const std::string &snmp_user, const std::string &securityLevel, const std::string &authPassPhrase, int snmp_version = 0, int timeout = 100): snmp_client(ip, port, community, snmp_user, securityLevel, "SHA", authPassPhrase,"","",snmp_version, timeout ) {};

        /** @brief Constructor for SNMP Service client.
         *  Uses the arguments provided to establish an snmp connection
         * @param ip The ip, as a string, for the tsc_client_service to establish an snmp communication with.
         * @param port Target port as integer on the host for snmp communication.
         * @param community The community id as a string. Defaults to "public" if unassigned.
         * @param snmp_user Security user used for SNMP authentication.
         * @param securityLevel Security level: authPriv or authNoPriv.
         * @param authProtocol The authentication protocol (MD5|SHA|SHA-224|SHA-256|SHA-384|SHA-512).
         * @param authPassPhrase The authentication protocol pass phrase.
         * @param privProtocol The privacy protocol (DES|AES|AES-192|AES-256).
         * @param privPassPhrase The privacy protocol pass phrase.
         * @param snmp_version The snmp_version as defined in net-snmp. Default to 0 if unassigned.
         *                     net-snmp version definition: SNMP_VERSION_1:0 SNMP_VERSION_2c:1 SNMP_VERSION_2u:2 SNMP_VERSION_3:3"
         * @param timeout The time in microseconds after which an snmp session request expires. Defaults to 100 if unassigned
         * **/
        snmp_client(const std::string &ip, const int &port, const std::string &community, const std::string &snmp_user, const std::string &securityLevel, const std::string &authProtocol, const std::string &authPassPhrase, const std::string &privProtocol, const std::string &privPassPhrase, int snmp_version = 3, int timeout = 100);

        /* Disable default copy constructor*/
        snmp_client(snmp_client &sc) = delete;

        /* Disable default move constructor*/
        snmp_client(snmp_client &&sc) = delete;

        /** @brief Returns true or false depending on whether the request could be processed for given input OID at the Traffic Signal Controller.
         *  @param input_oid The OID to request information for.
         *  @param request_type The request type for which the error is being logged. Accepted values are "GET" and "SET" only.
         *  @param value_int The integer value for the object returned by reference. For "SET" it is the value to be set.
         *  For "GET", it is the value returned for the returned object by reference.
         *  This is an optional argument, if not provided, defaults to 0.
         *  @param value_str String value for the object, returned by reference. Optional argument, if not provided the value is set as an empty string
         *  @return Integer value at the oid, returns false if value cannot be set/requested or oid doesn't have an integer value to return.*/

        virtual bool process_snmp_request(const std::string &input_oid, const request_type &request_type, snmp_response_obj &val);
        /** @brief Finds error type from status and logs an error.
         *  @param status The integer value corresponding to net-snmp defined errors. macros considered are STAT_SUCCESS(0) and STAT_TIMEOUT(2)
         *  @param request_type The request type for which the error is being logged (GET/SET).
         *  @param response The snmp_pdu struct */

        virtual int get_port() const; // Returns the current port (should always be 161 or 162)

        void log_error(const int &status, const request_type &request_type, const snmp_pdu *response) const;

        /** @brief Destructor for client. Closes the snmp session**/
        virtual ~snmp_client();
    };

} // namespace
