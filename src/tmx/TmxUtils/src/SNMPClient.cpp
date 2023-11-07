#include "SNMPClient.h"

namespace tmx::utils {
    
    // Client defaults to SNMPv3
    snmp_client::snmp_client(const std::string& ip, const int& port, const std::string& community, 
        const std::string &snmp_user, const std::string &securityLevel, const std::string &authPassPhrase, int snmp_version, int timeout)

        : ip_(ip), port_(port), community_(community),snmp_version_(snmp_version), timeout_(timeout)
    {
        
        PLOG(logDEBUG1) << "Starting SNMP Client";
        PLOG(logDEBUG1) << "Target device IP address: " << ip_;
        PLOG(logINFO) << "Target device NTCIP port: " << port_;

        // Bring the IP address and port of the target SNMP device in the required form, which is "IPADDRESS:PORT":
        std::string ip_port_string = ip_ + ":" + std::to_string(port_);    
        char* ip_port = &ip_port_string[0];
        
        // Initialize SNMP session parameters
        init_snmp("carma_snmp");
        snmp_sess_init(&session);
        session.peername = ip_port;
        session.version = snmp_version_; // SNMP_VERSION_3
        session.securityName = (char *)snmp_user.c_str();
        session.securityNameLen = snmp_user.length();

        session.securityModel = USM_SEC_MODEL_NUMBER;

        // Fallback behavior to setup a community for SNMP V1/V2
        if(snmp_version_ != 3){
            char community_char[community_.length()];
            std::copy(community_.begin(), community_.end(), community_char);
            unsigned char* comm = reinterpret_cast<unsigned char*>(community_char);
        
            session.community = comm;
            session.community_len = community_.length();
        }

        // SNMP authorization/privach config
        if (securityLevel == "authPriv") {
            session.securityLevel = SNMP_SEC_LEVEL_AUTHPRIV;
        }

        else if (securityLevel == "authNoPriv") {
            session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
        }

        else session.securityLevel = SNMP_SEC_LEVEL_NOAUTH;

        // Passphrase used for both authentication and privacy
        auto phrase_len = authPassPhrase.length();
        auto phrase = (u_char *) authPassPhrase.c_str();

        // Defining and generating auth config with SHA1
        session.securityAuthProto = usmHMACSHA1AuthProtocol;
        session.securityAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;
        session.securityAuthKeyLen = USM_AUTH_KU_LEN;

        if (generate_Ku(session.securityAuthProto,
                        session.securityAuthProtoLen,
                        phrase,phrase_len,
                        session.securityAuthKey,
                        &session.securityAuthKeyLen) != SNMPERR_SUCCESS)
        {
            std::string errMsg = "Error generating Ku from authentication pass phrase. \n";
            throw snmp_client_exception(errMsg);
        }

        // Defining and generating priv config with AES (since using SHA1)
        session.securityPrivProto = usmAESPrivProtocol;
        session.securityAuthProtoLen = USM_PRIV_PROTO_AES_LEN;
        session.securityPrivKeyLen = USM_PRIV_KU_LEN;

        if (generate_Ku(session.securityAuthProto,
                        session.securityAuthProtoLen,
                        phrase,phrase_len,
                        session.securityPrivKey,
                        &session.securityPrivKeyLen) != SNMPERR_SUCCESS)
        {
            std::string errMsg = "Error generating Ku from privacy pass phrase. \n";
            throw snmp_client_exception(errMsg);
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
    
    snmp_client::~snmp_client(){
        PLOG(logINFO) << "Closing SNMP session";
        snmp_close(ss);
    }


    // Original implementation used in Carma Streets https://github.com/usdot-fhwa-stol/snmp-client
    bool snmp_client::process_snmp_request(const std::string& input_oid, const request_type& request_type, snmp_response_obj& val){


        /*Structure to hold response from the remote host*/
        snmp_pdu *response;

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
        else{
            PLOG(logERROR) << "Invalid request type, method accpets only GET and SET";
        }

        // Read input OID into an OID variable:
        // net-snmp has several methods for creating an OID object
        // their documentation suggests using get_node. read_objid seems like a simpler approach
        // TO DO: investigate update to get_node
        if(!read_objid(input_oid.c_str(), OID, &OID_len)){
            // If oid cannot be created
            PLOG(logERROR) << "OID could not be created from input: " << input_oid;
            return false;
            
        }
        else{
            
            if(request_type == request_type::GET)
            {
                // Add OID to pdu for get request
                snmp_add_null_var(pdu, OID, OID_len);
            }
            else if(request_type == request_type::SET)
            {
                if(val.type == snmp_response_obj::response_type::INTEGER){
                    snmp_add_var(pdu, OID, OID_len, 'i', (std::to_string(val.val_int)).c_str());
                }
                // Needs to be finalized to support octet string use
                else if(val.type == snmp_response_obj::response_type::STRING){
                    PLOG(logERROR) << "Setting string value is currently not supported";
                    // std::string str_input(val.val_string.begin(), val.val_string.end());
                    // snmp_add_var(pdu, OID, OID_len, 's', str_input.c_str());
                    // return false;
                }
            }

            PLOG(logERROR) << "Created OID for input: " << input_oid;
        }
        // Send the request
        int status = snmp_synch_response(ss, pdu, &response);
        PLOG(logDEBUG) << "Response request status: " << status;

        // Check response
        if(status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
            
            PLOG(logINFO) << "STAT_SUCCESS, received a response";
            
            if(request_type == request_type::GET){
                for(auto vars = response->variables; vars; vars = vars->next_variable){
                    // Get value of variable depending on ASN.1 type
                    // Variable could be a integer, string, bitstring, ojbid, counter : defined here https://github.com/net-snmp/net-snmp/blob/master/include/net-snmp/types.h
                    // get Integer value
                    if(vars->type == ASN_INTEGER){
                        if(vars->val.integer){
                            val.type = snmp_response_obj::response_type::INTEGER;
                            val.val_int = *vars->val.integer;
                            PLOG(logDEBUG1) << "Integer value in object: " << val.val_int;
                        }
                        else{
                            PLOG(logERROR) << "Response specifies type integer, but no integer value found";
                            return false;
                        }
                        
                    }
                    else if(vars->type == ASN_OCTET_STR){
                        if(vars->val.string){
                            size_t str_len = vars->val_len;
                            for(size_t i = 0; i < str_len; ++i)
                            {
                                val.val_string.push_back(vars->val.string[i]);   
                            }
                            val.type = snmp_response_obj::response_type::STRING;
                            
                        }
                        else{
                            PLOG(logERROR) << "Response specifies type string, but no string value found";
                            return false;
                        }
                    }
                    else{
                        PLOG(logERROR) << "Received a message type which isn't an integer or string";
                        return false;
                    }
                }
            }
            else if(request_type == request_type::SET){
                
                if(val.type == snmp_response_obj::response_type::INTEGER){
                    PLOG(logDEBUG1) << "Success in SET for OID: " << input_oid << " Value: " << val.val_int;
                }

                else if(val.type == snmp_response_obj::response_type::STRING){
                    PLOG(logDEBUG1) << "Success in SET for OID: " << input_oid << " Value: ";
                    for(auto data : val.val_string){
                        PLOG(logDEBUG1) << data;
                    }
                }
            }
        
        }
        else 
        {
            log_error(status, request_type, response);
            return false;
        }

        if (response){
            snmp_free_pdu(response);
            OID_len = MAX_OID_LEN;
        }
        
        return true;
    }

    // Backup GET function for use with prexisting ERVCloudForwarding client
    std::string snmp_client::SNMPGet(const std::string &req_oid)
    {
        snmp_pdu *response;

        std::string result = "";
        auto pdu = snmp_pdu_create(SNMP_MSG_GET);
        
        if (!snmp_parse_oid(req_oid.c_str(), OID, &OID_len))
        {
            snmp_perror(req_oid.c_str());
            std::string errMsg = "OID could not be created from input:" + req_oid;
            throw snmp_client_exception(errMsg);
            SOCK_CLEANUP;
        }

        snmp_add_null_var(pdu, OID, OID_len);
        int status = snmp_synch_response(ss, pdu, &response);
        
        if (!response)
        {
            throw snmp_client_exception("No response for SNMP Get request!");
        }
        else if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
        {
            // SUCCESS: Return the response as result
            for (auto vars = response->variables; vars; vars = vars->next_variable)
            {
                if (vars->type == ASN_OCTET_STR)
                {
                    result = reinterpret_cast<char *>(vars->val.string);
                }
                else
                {
                    throw snmp_client_exception("Received respones type is not a string");
                }
            }
        }
        else
        {
            // FAILURE: Print what went wrong!
            std::string errMsg = snmp_errstring(response->errstat);
            throw snmp_client_exception("Error in packet. Reason:" + errMsg);
        }
        if (response)
            snmp_free_pdu(response);
        return result;
    }    


    int snmp_client::get_port() const
    {
        return port_;
    }


    void snmp_client::log_error(const int& status, const request_type& request_type, snmp_pdu *response) const
    {

        if (status == STAT_SUCCESS)
        {
            PLOG(logERROR) << "Variable type: " << response->variables->type;
            PLOG(logERROR) << "Error in packet " << static_cast<std::string>(snmp_errstring(static_cast<int>(response->errstat)));
        }
        else if (status == STAT_TIMEOUT){ 
        
            PLOG(logERROR) << "Timeout, no response from server";
        }
        else{
            if(request_type == request_type::GET){
                PLOG(logERROR) << "Unknown SNMP Error for GET";
            }
            else if(request_type == request_type::SET){
                PLOG(logERROR) << "Unknown SNMP Error for SET";
            }
        }
        
    }

} // namespace