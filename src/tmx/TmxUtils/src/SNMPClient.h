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

    struct snmp_request {
        std::string oid;
        /**
         * i(INTEGER),u(UNSIGNED),s(STRING),x(HEX STRING),d(DECIMAL STRING),n(NULLOBJ),o(OBJID),t(TIMETICKS),a(IPADDRESS),b(BITS)
         */
        char type;
        std::string value;
        std::string to_string() const {
            return oid + " " + type +  " " + value;
        };
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
    protected:
        /** @brief Default constructor for use by derived classes. 
         *         Needed by mock client for testing purposes.
        */
        snmp_client() = default;

    private:
        /*variables to store an snmp session*/
        // struct that holds information about who we're going to be talking to
        // We need to declare 2 of these, one to fill info with and second which is
        // a pointer returned by the library
        struct snmp_session session;
        struct snmp_session *ss = nullptr;
        /* Opaque single session handle for ss, obtained with snmp_sess_pointer. Used by the asynchronous
        request pump so that servicing this session does not touch every other session open in the process. */
        void *sessp_ = nullptr;


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
        int snmp_version_ = SNMP_VERSION_3; // default to 3
        /*Time after which the the snmp request times out*/
        int timeout_ = SNMP_DEFAULT_TIMEOUT; // default to 1s
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

        /**
         * @brief Helper method for building a single SET PDU holding every request.
         * @param requests The SET requests to encode.
         * @param request_log Human readable description of the PDU contents, filled in for logging.
         * @throws snmp_client_exception if any OID or value could not be added to the PDU.
         * @return A PDU owned by the caller. Ownership passes to net-snmp once the PDU is sent.
         */
        snmp_pdu *create_set_pdu(const std::vector<snmp_request> &requests, std::string &request_log) const;

        /**
         * @brief Helper method for handing a PDU to net-snmp without waiting for the device to answer.
         *        The outcome is reported later through async_response_callback.
         * @param pdu The PDU to send. Freed here if the send fails, owned by net-snmp otherwise.
         * @return True if the PDU was handed off, false if the send failed immediately.
         */
        bool send_pdu_async(snmp_pdu *pdu);

        /**
         * @brief Non blocking drain of this session. Dispatches callbacks for responses that have already
         *        arrived and expires requests whose timeout has elapsed, which is what frees the PDUs
         *        handed off by send_pdu_async. Never blocks.
         */
        void pump_async_responses() const;

        /**
         * @brief net-snmp callback for asynchronous requests. Logs the outcome and discards the response.
         *        Must not free the pdu, net-snmp frees it once this returns.
         */
        static int async_response_callback(int operation, snmp_session *sp, int reqid, snmp_pdu *pdu, void *magic);

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
         * @param timeout The time in microseconds after which an snmp session request expires. Defaults to 1s if unassigned.
         * **/
        snmp_client(const std::string &ip, const int &port, const std::string &community, const std::string &snmp_user, const std::string &securityLevel, const std::string &authPassPhrase, int snmp_version = 3, int timeout = SNMP_DEFAULT_TIMEOUT): snmp_client(ip, port, community, snmp_user, securityLevel, "SHA", authPassPhrase,"","",snmp_version, timeout ) {};

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
         * @param timeout The time in microseconds after which an snmp session request expires. Defaults to 1s if unassigned.
         * **/
        snmp_client(const std::string &ip, const int &port, const std::string &community, const std::string &snmp_user, const std::string &securityLevel, const std::string &authProtocol, const std::string &authPassPhrase, const std::string &privProtocol, const std::string &privPassPhrase, int snmp_version = 3, int timeout = SNMP_DEFAULT_TIMEOUT);

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
        
        /** @brief Sends a vector of snmpset requests as a single PDU
         *  @param requests A vector of snmp_request objects
         *  @return void
         */
        virtual bool process_snmp_set_requests(const std::vector<snmp_request> &requests);

        /** @brief Fire and forget version of process_snmp_set_requests. Sends the requests as a single PDU
         *  and returns as soon as it is written to the socket, so a slow or unresponsive device cannot stall
         *  the caller. The response, a timeout, or any other error is logged from async_response_callback
         *  during a later call into this client.
         *  @param requests A vector of snmp_request objects
         *  @return True if the PDU was handed off to net-snmp. This says nothing about whether the device
         *  received or accepted the SET.
         */
        virtual bool process_snmp_set_requests_async(const std::vector<snmp_request> &requests);

        /** @brief Sets how many times net-snmp retransmits a request that goes unanswered.
         *  0 means send once and never retransmit. A change applies to requests already in flight as well as future ones.
         *  @param retries Retransmission count, or SNMP_DEFAULT_RETRIES for the net-snmp default of 5.
         */
        void set_retries(int retries);
        /**
         * @brief Returns the current port
         */
        virtual int get_port() const; // Returns the current port (should always be 161 or 162)
        /**
         * @brief Returns the current ip address
         */
        virtual std::string get_ip() const; // Returns the current ip address

        void log_error(const int &status, const request_type &request_type, const snmp_pdu *response) const;

        /** @brief Destructor for client. Closes the snmp session**/
        virtual ~snmp_client();
    };

} // namespace
