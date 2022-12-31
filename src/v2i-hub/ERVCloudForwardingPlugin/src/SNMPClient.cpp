#include "SNMPClient.h"

SNMPClient::SNMPClient(const std::string &rsu_ip, uint16_t snmp_port, const std::string &securityUser, const std::string &authPassPhrase)
{
    std::string setCustomMibsCommand = "export MIBS=ALL";
    system(setCustomMibsCommand.c_str());
    std::string ip_port_string = rsu_ip + ":" + std::to_string(snmp_port);
    char *ip_port = &ip_port_string[0];
    init_snmp("snmpclient");
    snmp_sess_init(&session);
    session.peername = ip_port;
    session.version = SNMP_VERSION_3;
    session.securityName = strdup(securityUser.c_str());
    session.securityNameLen = strlen(session.securityName);
    session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
    session.securityAuthProto = snmp_duplicate_objid(usmHMACSHA1AuthProtocol, USM_AUTH_PROTO_SHA_LEN);
    session.securityAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;
    session.securityAuthKeyLen = USM_AUTH_KU_LEN;
    char *our_v3_passphrase = strdup(authPassPhrase.c_str());
    if (generate_Ku(session.securityAuthProto,
                    session.securityAuthProtoLen,
                    (u_char *)our_v3_passphrase, strlen(our_v3_passphrase),
                    session.securityAuthKey,
                    &session.securityAuthKeyLen) != SNMPERR_SUCCESS)
    {
        std::string errMsg = "Error generating Ku from authentication pass phrase. \n";
        throw SNMPClientException(errMsg);
    }
    ss = snmp_open(&session);
    if (!ss)
    {
        std::string errMsg = "Cannot open SNMP session. \n";
        throw SNMPClientException(errMsg);
    }
    else
    {
        fprintf(stdout, "snmp session is open.\n");
    }
}

std::string SNMPClient::SNMPGet(const std::string &req_oid)
{
    std::string result = "";
    netsnmp_pdu *response;
    auto pdu = snmp_pdu_create(SNMP_MSG_GET);
    if (!snmp_parse_oid(req_oid.c_str(), anOID, &anOID_len))
    {
        snmp_perror(req_oid.c_str());
        std::string errMsg = "OID could not be created from input:" + req_oid;
        throw SNMPClientException(errMsg);
        SOCK_CLEANUP;
    }
    snmp_add_null_var(pdu, anOID, anOID_len);
    auto status = snmp_synch_response(ss, pdu, &response);
    if (!response)
    {
        throw SNMPClientException("No response for SNMP Get request!");
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
                throw SNMPClientException("Received respones type is not a string");
            }
        }
    }
    else
    {
        // FAILURE: Print what went wrong!
        std::string errMsg = snmp_errstring(response->errstat);
        throw SNMPClientException("Error in packet. Reason:" + errMsg);
    }
    if (response)
        snmp_free_pdu(response);
    return result;
}

SNMPClient::~SNMPClient()
{
    fprintf(stdout, "Closing snmp session\n");
    snmp_close(ss);
}
