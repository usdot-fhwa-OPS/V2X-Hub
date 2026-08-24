#include "SNMPClient.h"
#include <tmx/messages/byte_stream.hpp>
#include <arpa/inet.h>

namespace tmx::utils
{
    // Client defaults to SNMPv3
    snmp_client::snmp_client(const std::string &ip, const int &port, const std::string &community,
                             const std::string &snmp_user, const std::string &securityLevel, const std::string &authProtocol, const std::string &authPassPhrase, const std::string &privProtocol, const std::string &privPassPhrase, int snmp_version, int timeout)

        : ip_(ip), port_(port), community_(community), snmp_version_(snmp_version), timeout_(timeout)
    {

        PLOG(logDEBUG3) << "String snmp_client configs : " << ip << " " << port << " " << community << " " << snmp_user << " " << securityLevel << " " << authProtocol << " " << authPassPhrase << " " << privProtocol << " " << privPassPhrase << " " << snmp_version << " " << timeout;

        // Validate IP address format
        struct in_addr addr4;
        struct in6_addr addr6;
        if (inet_pton(AF_INET, ip.c_str(), &addr4) != 1 && inet_pton(AF_INET6, ip.c_str(), &addr6) != 1)
        {
            throw snmp_client_exception("Invalid IP address: " + ip);
        }

        // Bring the IP address and port of the target SNMP device in the required form, which is "IPADDRESS:PORT":
        std::string ip_port_string = ip_ + ":" + std::to_string(port_);
        char *ip_port = &ip_port_string[0];

        // Initialize SNMP session parameters
        init_snmp("snmp_init");
        snmp_sess_init(&session);
        session.peername = ip_port;
        session.version = snmp_version_; // SNMP_VERSION_3
        // Fallback behavior to setup a community for SNMP V1/V2
        if (snmp_version_ != SNMP_VERSION_3)
        {
            session.community = reinterpret_cast<unsigned char *>(community_.data());
            session.community_len = community_.length();
            if (snmp_version_ == SNMP_VERSION_1)
            {
                session.securityModel = SNMP_SEC_MODEL_SNMPv1;
            }
            else
            {
                session.securityModel = SNMP_SEC_MODEL_SNMPv2c;
            }
        }
        else
        {
            session.securityName = (char *)snmp_user.c_str();
            session.securityNameLen = snmp_user.length();
            session.securityModel = SNMP_SEC_MODEL_USM;
        }

        // Set security level
        if (securityLevel == "authPriv")
        {
            session.securityLevel = SNMP_SEC_LEVEL_AUTHPRIV;
        }

        else if (securityLevel == "authNoPriv")
        {
            session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
        }
        else {
            session.securityLevel = SNMP_SEC_LEVEL_NOAUTH;
        }
        if (securityLevel == "authPriv" || securityLevel == "authNoPriv") {

            // Defining and generating authentication config
            const oid *usmAuthProto;
            if (authProtocol == "MD5") {
                usmAuthProto = usmHMACMD5AuthProtocol;
            }
            else if (authProtocol == "SHA") {
                usmAuthProto = usmHMACSHA1AuthProtocol;
            }
            else if (authProtocol == "SHA-224") {
                usmAuthProto = usmHMAC128SHA224AuthProtocol;
            }
            else if (authProtocol == "SHA-256") {
                usmAuthProto = usmHMAC192SHA256AuthProtocol;
            }
            else if (authProtocol == "SHA-384") {
                usmAuthProto = usmHMAC256SHA384AuthProtocol;
            }
            else if (authProtocol == "SHA-512") {
                usmAuthProto = usmHMAC384SHA512AuthProtocol;
            }
            else {
                throw snmp_client_exception("Invalid authentication protocol " + authProtocol + " !");
            }
            // Passphrase used for authentication
            auto authPhrase_len = authPassPhrase.length();
            auto authPhrase = (u_char *)authPassPhrase.c_str();
            // Note: This function allocates memory
            session.securityAuthProto = snmp_duplicate_objid(usmAuthProto, USM_AUTH_PROTO_SHA_LEN);
            session.securityAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;
            session.securityAuthKeyLen = USM_AUTH_KU_LEN;
            if (session.securityLevel != SNMP_SEC_LEVEL_NOAUTH && generate_Ku(session.securityAuthProto,
                                                                            session.securityAuthProtoLen,
                                                                            authPhrase, authPhrase_len,
                                                                            session.securityAuthKey,
                                                                            &session.securityAuthKeyLen) != SNMPERR_SUCCESS)
            {
                // Needed since snmp_duplicate_objid allocates memory
                free(session.securityAuthProto);
                throw snmp_client_exception("Error generating Ku from authentication pass phrase.");
            }
        }
        // Defining and generating privacy config
        if (securityLevel == "authPriv" ) {
            const oid *usmPrivProto;
            size_t privLen;
            if (privProtocol == "DES") {
                usmPrivProto = usmDESPrivProtocol;
                privLen = USM_PRIV_PROTO_DES_LEN;
            }
            else if (privProtocol == "AES") {
                usmPrivProto = usmAESPrivProtocol;
                privLen = USM_PRIV_PROTO_AES_LEN;
            }
            else if (privProtocol == "AES-128") {
                usmPrivProto = usmAES128PrivProtocol;
                privLen = USM_PRIV_PROTO_AES128_LEN;
            }
            else if (privProtocol == "AES-192") {
                usmPrivProto = usmAES192PrivProtocol;
                privLen = USM_PRIV_PROTO_AES192_LEN;
            }
            else if (privProtocol == "AES-256") {
                usmPrivProto = usmAES256PrivProtocol;
                privLen = USM_PRIV_PROTO_AES256_LEN;
            }
            else if (privProtocol == "AES-192-Cisco") {
                usmPrivProto = usmAES192CiscoPrivProtocol;
                privLen = USM_PRIV_PROTO_AES192_CISCO_LEN;
            }
            else if (privProtocol == "AES-256-Cisco") {
                usmPrivProto = usmAES256CiscoPrivProtocol;
                privLen = USM_PRIV_PROTO_AES256_CISCO_LEN;
            }
            else {
                // Needed since snmp_duplicate_objid allocates memory
                free(session.securityAuthProto);
                throw snmp_client_exception("Invalid privacy protocol " + privProtocol + " !");
            }
            // Passphrase used for privacy
            auto privPhrase_len = privPassPhrase.length();
            auto privPhrase = (u_char *)privPassPhrase.c_str();
            // NOTE : This function allocates memory
            session.securityPrivProto = snmp_duplicate_objid(usmPrivProto, privLen);
            session.securityPrivProtoLen = privLen;
            session.securityPrivKeyLen = USM_PRIV_KU_LEN;
            if (session.securityLevel == SNMP_SEC_LEVEL_AUTHPRIV && generate_Ku(session.securityAuthProto,
                                                                                session.securityAuthProtoLen,
                                                                                privPhrase, privPhrase_len,
                                                                                session.securityPrivKey,
                                                                                &session.securityPrivKeyLen) != SNMPERR_SUCCESS)
            {
                // Needed since snmp_duplicate_objid allocates memory
                free(session.securityAuthProto);
                free(session.securityPrivProto);
                throw snmp_client_exception("Error generating Ku from privacy pass phrase.");
            }
        }
        session.timeout = timeout_;

        // Opens the snmp session if it exists
        ss = snmp_open(&session);
        // Needed since snmp_duplicate_objid allocates memory        
        free(session.securityAuthProto);
        free(session.securityPrivProto);
        

        if (ss == nullptr)
        {
            PLOG(logERROR) << "Failed to establish session with target device";
            snmp_sess_perror("snmpget", &session);
            throw snmp_client_exception("Failed to establish session with target device");
        }
        else
        {
            // Opaque handle for this session only. The traditional snmp_read/snmp_timeout calls service
            // every session open in the process, the snmp_sess_* equivalents service just this one.
            sessp_ = snmp_sess_pointer(ss);
            PLOG(logINFO) << "Established session with device at " << ip_;
        }
    }

    snmp_client::~snmp_client()
    {
        PLOG(logINFO) << "Closing SNMP session";
        if (ss)
        {
            // Frees any asynchronous request still pending on this session. The callbacks are not invoked,
            // but they carry no state of ours so nothing is leaked by that.
            snmp_close(ss);
            ss = nullptr;
            sessp_ = nullptr;
        }
    }

    snmp_pdu *snmp_client::create_set_pdu(const std::vector<snmp_request> &requests, std::string &request_log) const {
        int failures = 0;
        /*Structure to hold all of the information that we're going to send to the remote host*/
        struct snmp_pdu *pdu = snmp_pdu_create(SNMP_MSG_SET);
        request_log = "Outgoing Request :";
        for (const auto &request : requests) {
            request_log.append("\n" + request.to_string());
            // Local OID buffer per request, since snmp_parse_oid uses the length both as the capacity of the
            // buffer going in and as the length of the parsed OID coming out.
            oid request_oid[MAX_OID_LEN];
            size_t request_oid_len = MAX_OID_LEN;
            if (snmp_parse_oid(request.oid.c_str(), request_oid, &request_oid_len) == nullptr) {
                snmp_perror("snmp_parse_oid");
                PLOG(logERROR) << "OID could not be created from input: " << request.oid;
                failures++;
                continue;
            }
            if (snmp_add_var(pdu, request_oid, request_oid_len, request.type, request.value.c_str())) {
                snmp_perror("snmp_add_var");
                PLOG(logERROR) << "PDU could not be created from input: " << request.oid;
                failures++;
            }
        }
        if (failures > 0) {
            snmp_free_pdu(pdu);
            throw snmp_client_exception("Encountered " + std::to_string(failures) + " failures while creating PDU");
        }
        return pdu;
    }

    int snmp_client::async_response_callback(int operation, snmp_session *sp, int reqid, snmp_pdu *pdu, void *magic) {
        // NOTE: net-snmp owns the pdu here and frees it once this returns. Do not free it.
        if (operation == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE) {
            if (pdu && pdu->errstat == SNMP_ERR_NOERROR) {
                PLOG(logDEBUG3) << "Asynchronous SNMP request " << reqid << " acknowledged by device";
            }
            else {
                PLOG(logWARNING) << "Asynchronous SNMP request " << reqid << " rejected by device: "
                                 << (pdu ? snmp_errstring(pdu->errstat) : "no response pdu");
            }
        }
        else if (operation == NETSNMP_CALLBACK_OP_RESEND) {
            // Routine retransmission after a missed response, not a failure. net-snmp reports one of these
            // per retry before it gives up and reports the timeout below.
            PLOG(logDEBUG3) << "Asynchronous SNMP request " << reqid << " not answered in time, retransmitting";
        }
        else if (operation == NETSNMP_CALLBACK_OP_TIMED_OUT) {
            PLOG(logWARNING) << "Asynchronous SNMP request " << reqid << " timed out, dropping it.";
        }
        else if (operation == NETSNMP_CALLBACK_OP_SEND_FAILED) {
            PLOG(logWARNING) << "Asynchronous SNMP request " << reqid << " could not be sent, dropping it.";
        }
        else if (operation == NETSNMP_CALLBACK_OP_SEC_ERROR) {
            PLOG(logWARNING) << "Asynchronous SNMP request " << reqid << " failed security processing, dropping it.";
        }
        else {
            PLOG(logWARNING) << "Asynchronous SNMP request " << reqid << " ended with callback operation " << operation;
        }
        return 1;
    }

    void snmp_client::pump_async_responses() const {
        if (!sessp_) {
            return;
        }
        int numfds = 0;
        fd_set fdset;
        FD_ZERO(&fdset);
        int block = 0;
        // Zeroed out with block false, so net-snmp cannot ask us to wait on its behalf
        struct timeval timeout = {0, 0};
        snmp_sess_select_info(sessp_, &numfds, &fdset, &timeout, &block);
        if (numfds > 0) {
            struct timeval no_wait = {0, 0};
            if (select(numfds, &fdset, nullptr, nullptr, &no_wait) > 0) {
                // Dispatches async_response_callback for every response already waiting on the socket
                snmp_sess_read(sessp_, &fdset);
            }
        }
        // Expires requests whose timeout has elapsed, which is what frees the PDUs we handed off
        snmp_sess_timeout(sessp_);
    }

    bool snmp_client::send_pdu_async(snmp_pdu *pdu) {
        if (!ss) {
            PLOG(logERROR) << "Cannot send asynchronous SNMP request, no open session";
            snmp_free_pdu(pdu);
            return false;
        }
        // Drain first, so responses and timeouts from earlier sends are reaped before another is added
        pump_async_responses();

        int reqid = snmp_async_send(ss, pdu, snmp_client::async_response_callback, nullptr);
        if (reqid == 0) {
            // net-snmp only takes ownership of the PDU when the send succeeds
            snmp_sess_perror("snmp_async_send", ss);
            PLOG(logERROR) << "Failed to send asynchronous SNMP request to " << ip_;
            snmp_free_pdu(pdu);
            return false;
        }
        PLOG(logDEBUG3) << "Sent asynchronous SNMP request " << reqid << " to " << ip_ << ", not waiting for a response";
        return true;
    }

    bool snmp_client::process_snmp_set_requests_async(const std::vector<snmp_request> &requests) {
        FILE_LOG(logDEBUG3) << "Sending SNMP Requests length " << requests.size() << " asynchronously";
        std::string request_log;
        struct snmp_pdu *pdu = create_set_pdu(requests, request_log);
        PLOG(logDEBUG3) << request_log;
        return send_pdu_async(pdu);
    }

    bool snmp_client::process_snmp_set_requests(const std::vector<snmp_request> &requests) {
        /*Structure to hold response from the remote host*/
        struct snmp_pdu *response;
        FILE_LOG(logDEBUG3) << "Sending SNMP Requests length " << requests.size();
        std::string request_log;
        struct snmp_pdu *pdu = create_set_pdu(requests, request_log);
        int status = snmp_synch_response(ss, pdu, &response);
        PLOG(logDEBUG3) << request_log; 
        PLOG(logDEBUG3) << "Response request status: " << status << " (=" << (status == STAT_SUCCESS ? "SUCCESS" : "FAILED") << ")";
        bool success = false;
        // Check GET response
        if (status == STAT_SUCCESS && response && response->errstat == SNMP_ERR_NOERROR ) {
            success = true;
            for (auto vars = response->variables; vars;
                     vars = vars->next_variable) {
                if (tmx::utils::FILELog::ReportingLevel() >= tmx::utils::logDEBUG3) {
                    print_variable(vars->name, vars->name_length, vars);
                }
            }
            
        } 
        else {
            log_error(status, request_type::SET, response);
        }

        if (response)
        {
            snmp_free_pdu(response);
            OID_len = MAX_OID_LEN;
        }
        return success;



    }
    // Original implementation used in Carma Streets https://github.com/usdot-fhwa-stol/snmp-client
    bool snmp_client::process_snmp_request(const std::string &input_oid, const request_type &request_type, snmp_response_obj &val)
    {  
        /*Structure to hold all of the information that we're going to send to the remote host*/
        struct snmp_pdu *pdu;
        /*Structure to hold response from the remote host*/
        struct snmp_pdu *response;
        // Create pdu for the data
        if (request_type == request_type::GET)
        {
            PLOG(logDEBUG3) << "Attempting to GET value for: " << input_oid;
            pdu = snmp_pdu_create(SNMP_MSG_GET);
        }
        else if (request_type == request_type::SET)
        {
            pdu = snmp_pdu_create(SNMP_MSG_SET);
        }
        else
        {
            PLOG(logERROR) << "Invalid request type, method accepts only GET and SET";
            return false;
        }

        // Read input OID into an OID variable:
        // net-snmp has several methods for creating an OID object
        // their documentation suggests using get_node. read_objid seems like a simpler approach
        // TO DO: investigate update to get_node
        if (!snmp_parse_oid(input_oid.c_str(), OID, &OID_len)) {
            snmp_perror("snmp_parse_oid");
            PLOG(logERROR) << "OID could not be created from input: " << input_oid;
            snmp_free_pdu(pdu);
            snmp_close(ss);
            return false;
        }
        else
        {
            if (request_type == request_type::GET)
            {
                // Add OID to pdu for get request
                snmp_add_null_var(pdu, OID, OID_len);
            }
            else if (request_type == request_type::SET)
            {
                if (val.type == snmp_response_obj::response_type::INTEGER)
                {
                    PLOG(logDEBUG3) << "Attempting to SET value: " << val.val_int << " for OID: " << input_oid;
                    snmp_add_var(pdu, OID, OID_len, 'i', (std::to_string(val.val_int)).c_str());
                }
                else if (val.type == snmp_response_obj::response_type::STRING)
                {
                    PLOG(logDEBUG3) << "Attempting to SET octet string: "
                                    << tmx::byte_stream_encode(tmx::byte_stream(val.val_string.begin(), val.val_string.end()))
                                    << " (" << val.val_string.size() << " bytes)"
                                    << " for OID: " << input_oid;
                    snmp_pdu_add_variable(pdu, OID, OID_len, ASN_OCTET_STR, val.val_string.data(), val.val_string.size());
                }
            }

            PLOG(logDEBUG3) << "Created OID for input: " << input_oid;
        }
        // Send the request
        int status = snmp_synch_response(ss, pdu, &response);
        PLOG(logDEBUG3) << "Response request status: " << status << " (=" << (status == STAT_SUCCESS ? "SUCCESS" : "FAILED") << ")";

        // Check GET response
        if (status == STAT_SUCCESS && response && response->errstat == SNMP_ERR_NOERROR )
        {
            if ( request_type == request_type::GET ) {
                process_snmp_get_response(val, *response);
            }
            else if( request_type == request_type::SET){
                process_snmp_set_response(val, input_oid);
            }
            else {
                log_error(status, request_type, response);
                return false;
            }
        }
        else
        {
            log_error(status, request_type, response);
            return false;
        }

        if (response)
        {
            snmp_free_pdu(response);
            OID_len = MAX_OID_LEN;
        }

        return true;
    }

    void snmp_client::set_retries(int retries) {
        if (!ss) {
            PLOG(logERROR) << "Cannot set SNMP retries, no open session";
            return;
        }
        ss->retries = retries;
        PLOG(logINFO) << "SNMP retries for " << ip_ << " set to " << retries;
    }

    int snmp_client::get_port() const
    {
        return port_;
    }

    std::string snmp_client::get_ip() const
    {
        return ip_;
    }

    void snmp_client::process_snmp_get_response(snmp_response_obj &val,  const snmp_pdu &response) const {
        /*Structure to hold all of the information that we're going to send to the remote host*/
        for (auto vars = response.variables; vars; vars = vars->next_variable)
        {
            // Get value of variable depending on ASN.1 type
            // Variable could be a integer, string, bitstring, ojbid, counter : defined here https://github.com/net-snmp/net-snmp/blob/master/include/net-snmp/types.h
            // get Integer value
            if (vars->type == ASN_INTEGER && vars->val.integer)
            {
                val.type = snmp_response_obj::response_type::INTEGER;
                val.val_int = *vars->val.integer;
            }
            else if (vars->type == ASN_OCTET_STR && vars->val.string)
            {
                size_t str_len = vars->val_len;
                for (size_t i = 0; i < str_len; ++i)
                {
                    val.val_string.push_back(vars->val.string[i]);
                }
                val.type = snmp_response_obj::response_type::STRING;
            }
        }
    }

    void snmp_client::process_snmp_set_response( const snmp_response_obj &val,  const std::string &input_oid) const {
        if(val.type == snmp_response_obj::response_type::INTEGER){
            FILE_LOG(logDEBUG3) << "Success in SET for OID: " << input_oid << " Value: " << val.val_int << std::endl;
        }

        else if(val.type == snmp_response_obj::response_type::STRING){
            FILE_LOG(logDEBUG3) << "Success in SET for OID: " << input_oid
                << " Value: " << tmx::byte_stream_encode(tmx::byte_stream(val.val_string.begin(), val.val_string.end()))
                << " (" << val.val_string.size() << " bytes)" << std::endl;
        }
    }

    void snmp_client::log_error(const int &status, const request_type &request_type, const snmp_pdu *response) const
    {

        if (status == STAT_SUCCESS)
        {
            PLOG(logERROR) << "Variable type: " << response->variables->type << ". Error in packet " << static_cast<std::string>(snmp_errstring(static_cast<int>(response->errstat)));
        }
        else if (status == STAT_TIMEOUT)
        {
            PLOG(logERROR) << "Timeout, no response from server";
        }
        else
        {
            PLOG(logERROR) << "Unknown SNMP Error (status code: " << status << ") for " << (request_type == request_type::GET ? "GET" : "SET");
        }

        if (response)
        {
            PLOG(logERROR) << "SNMP error in response: " << snmp_errstring(response->errstat);
        }

    }
} // namespace
