#include "SNMPWorker.h"

namespace ImmediateForward {
    void clearImmediateForwardTable(const std::unique_ptr<tmx::utils::snmp_client> &client) {

    }

    std::unordered_map<std::string, unsigned int> initializeImmediateForwardTable(const std::unique_ptr<tmx::utils::snmp_client> &client, const std::vector<Message> &messages){
        std::unordered_map<std::string, unsigned int> tmxMessageTypeToIMFTableIndex;
        return tmxMessageTypeToIMFTableIndex;
    }

}