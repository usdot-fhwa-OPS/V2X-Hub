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
        auto local_ip = std::getenv(LOCAL_IP);
        if ( local_ip ) {
            return local_ip;
        }
        else {
            throw TmxException("Local IP environment variable " + std::string(LOCAL_IP) + " not set!");
        }
    }

    std::string get_environment_variable(const char *config_name, bool required) {
        if ( config_name ) {
            if ( std::getenv(config_name) != nullptr) {
                std::string config =  std::getenv(config_name);
                return config;
            }
            else {
                std::string config_name_str = config_name;
                if ( required ) {
                    throw TmxException("Required environment variable " + config_name_str + " not set!");
                }
                return "";
            }
        }
        else {
            throw TmxException("Environment variable name cannot be null!");
        }
    } 
}