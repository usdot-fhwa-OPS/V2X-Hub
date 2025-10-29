#include "SpatMode.h"

namespace SpatPlugin {
    SPAT_MODE spat_mode_from_string(const std::string &mode) {
        for (const auto& [key, value]: spat_mode_map ) {
            if ( value == mode) {
                return key;
            }
        }
        return SPAT_MODE::UNKNOWN;
    }

    std::string spat_mode_to_string(const SPAT_MODE &mode) {
        auto ret = spat_mode_map.find( mode);
        if (ret != spat_mode_map.end()){
            return ret->second;
        }
        return "UNKNOWN";

    }
}