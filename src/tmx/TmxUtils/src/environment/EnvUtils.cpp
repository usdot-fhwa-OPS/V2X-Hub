#include "environment/EnvUtils.h"

namespace tmx::utils::environment{
    bool is_simulation_mode() {
        auto sim_mode = std::getenv(SIMULATION_MODE);
        if ( sim_mode) {
            std::string sim_mode_string = sim_mode;
            if ( sim_mode_string.compare("true") == 0 || sim_mode_string.compare("TRUE") == 0) {
                return true;
            }
        }
        return false;

    }
    
    std::string get_local_ip() {
        auto local_ip = std::getenv(V2XHUB_IP);
        if ( local_ip ) {
            return local_ip;
        }
        else {
            throw TmxException("Local IP environment variable " + std::string(V2XHUB_IP) + " not set!");
        }
    }

    std::string get_environment_variable(const char *env_name, bool required) {
        if ( env_name ) {
            if ( std::getenv(env_name) != nullptr) {
                std::string env_value =  std::getenv(env_name);
                return env_value;
            }
            else {
                if ( required ) {
                    throw TmxException("Required environment variable " + std::string(env_name) + " not set!");
                }
                return "";
            }
        }
        else {
            throw TmxException("Environment variable name cannot be null!");
        }
    } 
}