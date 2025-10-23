#pragma once
#include <string>
#include <map>
namespace SpatPlugin {
    /**
     * Enumeration to capture desired format of received SPAT
     */
    enum class SPAT_MODE {
        SPAT = 0,
        TSCBM = 1,
        UNKNOWN = 2
    };
    /**
     * Map of SPAT_MODE enumerations to translate between string and enum values
     */
    const std::map<SPAT_MODE, std::string> spat_mode_map = {
        {SPAT_MODE::SPAT, "SPAT"},
        {SPAT_MODE::TSCBM, "TSCBM"},
        {SPAT_MODE::UNKNOWN, "UNKNOWN"}};
    
    /**
     * @brief Convert std::string to SPAT_MODE enumeration value
     * @param string representation of SPAT_MODE enumeration
     * @returns SPAT_MODE enumeration
     */
    SPAT_MODE spat_mode_from_string( const std::string &mode );
    /**
     * @brief Convert SPAT_MODE enumeration to string representation
     * @param SPAT_MODE enumeration 
     * @returns string representation
     */
    std::string spat_mode_to_string( const SPAT_MODE &mode );
}