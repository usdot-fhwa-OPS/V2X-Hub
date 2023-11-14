#include "SNMPClientException.h"

namespace tmx::utils {

    snmp_client_exception::snmp_client_exception(const std::string &msg): std::runtime_error(msg){};

    snmp_client_exception::~snmp_client_exception() = default;
}