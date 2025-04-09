#include "simulation/SimulationEnvUtils.h"

namespace tmx::utils::sim{
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

    std::string get_sim_config(const char *config_name, bool required) {
        if ( std::getenv(config_name) != nullptr) {
            std::string config =  std::getenv(config_name);
            return config;
        }
        else {
            std::string config_name_str = config_name;
            if ( required ) {
                throw TmxException("Required simulation config " + config_name_str + " not set!");
            }
        }
        return "";
    } 
}