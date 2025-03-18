#pragma once

#include <unordered_map>
#include <string>
#include <tmx/TmxException.hpp>

namespace tmx::utils::rsu
{
    enum class RSU_SPEC {
        RSU_4_1 = 0,
        NTCIP_1218 = 1
    };

    /**
     * Map to conver between string RSU spec and enumeration
     */
    const static std::unordered_map<std::string, RSU_SPEC> stringToRSUSpecMap = {
        { "RSU4.1", RSU_SPEC::RSU_4_1},
        { "NTCIP1218", RSU_SPEC::NTCIP_1218}    
    };

     /**
     * Helper function to convert RSU Spec enumeration to string
     */
    std::string rsuSpecToString(const RSU_SPEC &spec);

    /**
     * Helper function to convert string to RSUS Spec enumeration
     */
    RSU_SPEC stringToRSUSpec( const std::string &spec);
}