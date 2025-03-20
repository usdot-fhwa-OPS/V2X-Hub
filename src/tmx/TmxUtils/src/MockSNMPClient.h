#pragma once
#include "SNMPClient.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace tmx::utils;
using namespace std;

namespace unit_test
{
    class mock_snmp_client : public snmp_client
    {
    public:
        mock_snmp_client(const std::string &ip, const int &port, const std::string &community, const std::string &snmp_user, const std::string &securityLevel, const std::string &authPassPhrase, int snmp_version = 3, int timeout = 100) : snmp_client(ip, port, community, snmp_user, securityLevel, authPassPhrase, snmp_version, timeout){};
        mock_snmp_client(const std::string &ip, const int &port, const std::string &community, const std::string &snmp_user, const std::string &securityLevel, const std::string &authProtocol, const std::string &authPassPhrase, const std::string &privProtocol, const std::string &privPassPhrase, int snmp_version = 3, int timeout = 100) : snmp_client(ip, port, community, snmp_user, securityLevel, authProtocol, authPassPhrase, privProtocol, privPassPhrase, snmp_version, timeout){};
        ~mock_snmp_client() = default;
        MOCK_METHOD(bool, process_snmp_request, (const std::string &input_oid, const request_type &request_type, snmp_response_obj &val), (override));
        MOCK_METHOD(bool, process_snmp_set_requests,(const std::vector<snmpRequest> &requests), (override));
    };
}