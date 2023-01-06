
#ifndef SNMPCLIENT_H_
#define SNMPCLIENT_H_

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/utilities.h>
#include <net-snmp/net-snmp-includes.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <SNMPClientException.h>

class SNMPClient
{
private:
    netsnmp_session session;
    netsnmp_session *ss;
    oid anOID[MAX_OID_LEN];
    size_t anOID_len = MAX_OID_LEN;

public:
    /**
     * @brief Construct a new SNMPClient object
     * @param ip RSU IP
     * @param port SNMP port
     */
    SNMPClient(const std::string &rsu_ip, uint16_t snmp_port, const std::string &securityUser, const std::string &authPassPhrase);
    /**
     * @brief Send SNMP v3 Get request to an RSU to retrieve data
     * @param oid  OID (Object Identifier) uniquely identify managed objects in a MIB database. Concept refers to: https://en.wikipedia.org/wiki/Management_information_base
     * @return std::string identified by the oid. If SNMP response is not string, exit with failure.
     */
    std::string SNMPGet(const std::string &oid);
    ~SNMPClient();
};
#endif
