#include "RSUSpec.h"

namespace tmx::utils::rsu {
    RSU_SPEC stringToRSUSpec(const std::string &spec) {
        try {
            return stringToRSUSpecMap.at(spec);
        }
        catch (const std::out_of_range& ) {
            throw tmx::TmxException("RSU Specification " + spec + " is not supported!"); 

        }
    }

    std::string rsuSpecToString(const RSU_SPEC &spec) {
        for (auto const &[name, m] : stringToRSUSpecMap){
                if (spec == m) {
                    return name;
                }
        }
        throw tmx::TmxException("RSU Specification is not supported!"); 
    }
}