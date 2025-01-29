#include "SNMPClient.h"

namespace tmx::utils
{
    // Client defaults to SNMPv3
    snmp_client::snmp_client(const std::string &ip, const int &port, const std::string &community,
                             const std::string &snmp_user, const std::string &securityLevel, const std::string &authProtocol, const std::string &authPassPhrase, const std::string &privProtocol, const std::string &privPassPhrase, int snmp_version, int timeout)

        : ip_(ip), port_(port), community_(community), snmp_version_(snmp_version), timeout_(timeout)
    {

        PLOG(logDEBUG1) << "String snmp_client configs : " << ip << " " << port << " " << community << " " << snmp_user << " " << securityLevel << " " << authProtocol << " " << authPassPhrase << " " << privProtocol << " " << privPassPhrase << " " << snmp_version << " " << timeout;

        // Bring the IP address and port of the target SNMP device in the required form, which is "IPADDRESS:PORT":
        std::string ip_port_string = ip_ + ":" + std::to_string(port_);
        char *ip_port = &ip_port_string[0];

        // Initialize SNMP session parameters
        init_snmp("snmp_init");
        snmp_sess_init(&session);
        session.peername = ip_port;
        session.version = snmp_version_; // SNMP_VERSION_3
        session.securityName = (char *)snmp_user.c_str();
        session.securityNameLen = snmp_user.length();
        session.securityModel = SNMP_SEC_MODEL_USM;

        // Fallback behavior to setup a community for SNMP V1/V2
        if (snmp_version_ != SNMP_VERSION_3)
        {
            session.community = (unsigned char *)community.c_str();
            session.community_len = community_.length();
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
            else usmAuthProto = usmHMACSHA1AuthProtocol;
            // Passphrase used for authentication
            auto authPhrase_len = authPassPhrase.length();
            auto authPhrase = (u_char *)authPassPhrase.c_str();

            session.securityAuthProto = snmp_duplicate_objid(usmAuthProto, USM_AUTH_PROTO_SHA_LEN);
            session.securityAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;
            session.securityAuthKeyLen = USM_AUTH_KU_LEN;
            if (session.securityLevel != SNMP_SEC_LEVEL_NOAUTH && generate_Ku(session.securityAuthProto,
                                                                            session.securityAuthProtoLen,
                                                                            authPhrase, authPhrase_len,
                                                                            session.securityAuthKey,
                                                                            &session.securityAuthKeyLen) != SNMPERR_SUCCESS)
            {
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
            else if (securityLevel == "authPriv" ) {
                throw snmp_client_exception("Invalid privacy protocol " + privProtocol + " !");
            }
            // Passphrase used for privacy
            auto privPhrase_len = privPassPhrase.length();
            auto privPhrase = (u_char *)privPassPhrase.c_str();

            session.securityPrivProto = snmp_duplicate_objid(usmPrivProto, privLen);
            session.securityPrivProtoLen = privLen;
            session.securityPrivKeyLen = USM_PRIV_KU_LEN;
            if (session.securityLevel == SNMP_SEC_LEVEL_AUTHPRIV && generate_Ku(session.securityAuthProto,
                                                                                session.securityAuthProtoLen,
                                                                                privPhrase, privPhrase_len,
                                                                                session.securityPrivKey,
                                                                                &session.securityPrivKeyLen) != SNMPERR_SUCCESS)
            {
                throw snmp_client_exception("Error generating Ku from privacy pass phrase.");
            }
        }
        session.timeout = timeout_;

        // Opens the snmp session if it exists
        ss = snmp_open(&session);

        if (ss == nullptr)
        {
            PLOG(logERROR) << "Failed to establish session with target device";
            snmp_sess_perror("snmpget", &session);
            throw snmp_client_exception("Failed to establish session with target device");
        }
        else
        {
            PLOG(logINFO) << "Established session with device at " << ip_;
        }
    }

    snmp_client::~snmp_client()
    {
        PLOG(logINFO) << "Closing SNMP session";
        snmp_close(ss);
    }
    void snmp_client::process_snmp_set_requests(const std::vector<snmpRequest> &requests) {
        int failures = 0;
        pdu = snmp_pdu_create(SNMP_MSG_SET);
        for (const auto request : requests) {
            if (snmp_parse_oid(request.oid.c_str(), OID, &OID_len) == NULL) {
                snmp_perror("snmp_parse_oid");
                PLOG(logERROR) << "OID could not be created from input: " << request.oid;
                failures++;
            }
            if (snmp_add_var(pdu, OID, OID_len, request.type, request.value.c_str())) {
                snmp_perror("snmp_add_var");
                PLOG(logERROR) << "PDU could not be created from input: " << request.oid;
                failures++;
            }  
        }
        if (failures > 0) {
            snmp_close(ss);
            throw snmp_client_exception("Encountered " + std::to_string(failures) + " failures while creating PDU");
        }
        int status = snmp_synch_response(ss, pdu, &response);
        PLOG(logDEBUG) << "Response request status: " << status << " (=" << (status == STAT_SUCCESS ? "SUCCESS" : "FAILED") << ")";

        // Check GET response
        if (status == STAT_SUCCESS && response && response->errstat == SNMP_ERR_NOERROR ) {
            for (auto vars = response->variables; vars;
                     vars = vars->next_variable) {
                print_variable(vars->name, vars->name_length, vars);
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


        
    }
    // Original implementation used in Carma Streets https://github.com/usdot-fhwa-stol/snmp-client
    bool snmp_client::process_snmp_request(const std::string &input_oid, const request_type &request_type, snmp_response_obj &val)
    {
        // Create pdu for the data
        if (request_type == request_type::GET)
        {
            PLOG(logDEBUG1) << "Attempting to GET value for: " << input_oid;
            pdu = snmp_pdu_create(SNMP_MSG_GET);
        }
        else if (request_type == request_type::SET)
        {
            PLOG(logDEBUG1) << "Attempting to SET value for " << input_oid << " to " << val.val_int;
            pdu = snmp_pdu_create(SNMP_MSG_SET);
        }
        else
        {
            PLOG(logERROR) << "Invalid request type, method accpets only GET and SET";
            return false;
        }

        // Read input OID into an OID variable:
        // net-snmp has several methods for creating an OID object
        // their documentation suggests using get_node. read_objid seems like a simpler approach
        // TO DO: investigate update to get_node
        if (!snmp_parse_oid(input_oid.c_str(), OID, &OID_len)) {
            snmp_perror("snmp_parse_oid");
            PLOG(logERROR) << "OID could not be created from input: " << input_oid;
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
                    snmp_add_var(pdu, OID, OID_len, 'i', (std::to_string(val.val_int)).c_str());
                }
                // Needs to be finalized to support octet string use
                else if (val.type == snmp_response_obj::response_type::STRING)
                {
                    std::string val_string (val.val_string.begin(), val.val_string.end());
                    snmp_add_var(pdu, OID, OID_len, 'x', val_string.c_str());
                }
            }

            PLOG(logDEBUG) << "Created OID for input: " << input_oid;
        }
        // Send the request
        int status = snmp_synch_response(ss, pdu, &response);
        PLOG(logDEBUG) << "Response request status: " << status << " (=" << (status == STAT_SUCCESS ? "SUCCESS" : "FAILED") << ")";

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

    int snmp_client::get_port() const
    {
        return port_;
    }

    std::string snmp_client::get_ip() const
    {
        return ip_;
    }

    void snmp_client::process_snmp_get_response(snmp_response_obj &val,  const snmp_pdu &response) const {
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
            FILE_LOG(logDEBUG) << "Success in SET for OID: " << input_oid << " Value: " << val.val_int << std::endl;
        }

        else if(val.type == snmp_response_obj::response_type::STRING){
            FILE_LOG(logDEBUG) << "Success in SET for OID: " << input_oid << " Value:" << std::endl;
            for(auto data : val.val_string){
                FILE_LOG(logDEBUG) <<  data ;
            }
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
