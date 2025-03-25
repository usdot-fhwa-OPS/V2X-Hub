#pragma once
#include <string>
namespace tmx::utils::rsu::mib::rsu41
{
    /**
     * @brief This header file contains a subset of RSU MIB definition from https://github.com/certificationoperatingcouncil/COC_TestSpecs/blob/master/AppNotes/RSU/RSU-MIB.txt
     */
    // Contains the ID given to this RSU.
    static const std::string  RSU_ID_OID = "1.0.15628.4.1.17.4.0";

    // Contains the version of this MIB supported by this RSU, e.g. rsuMIB 4.1 rev201812060000Z
    static const std::string  RSU_MIB_VERSION = "1.0.15628.4.1.17.1.0";

    // Contains the version of firmware running on this RSU.
    static const std::string  RSU_FIRMWARE_VERSION = "1.0.15628.4.1.17.2.0";

    // Contains the name of the manufacturer of this RSU.
    static const std::string  RSU_MANUFACTURER = "1.0.15628.4.1.17.5.0";

    // Contains GPS NMEA GPGGA output string.
    static const std::string  RSU_GPS_OUTPUT_STRING = "1.0.15628.4.1.8.5.0";

    // Immediate Forward Message Index
    static const std::string  RSU_IFM_INDEX = "1.0.15628.4.1.5.1.1.0";

    // Immediate Forward Message PSID.
    static const std::string  RSU_IFM_PSID = "1.0.15628.4.1.5.1.2.0";

    // Immediate Forward Message DSRC Message ID
    static const std::string  RSU_IFM_DSRC_MSG_ID = "1.0.15628.4.1.5.1.3.0";

    // Immediate Forward Message Transmit Mode
    static const std::string  RSU_IFM_TX_MODE = "1.0.15628.4.1.5.1.4.0";

    // DSRC channel set for Immediate Forward Message transmit
    static const std::string  RSU_IFM_TX_CHANNEL = "1.0.15628.4.1.5.1.5.0";

    // Set this bit to enable transmission of the message 0=off, 1=on
    static const std::string  RSU_IFM_ENABLE = "1.0.15628.4.1.5.1.6.0";

    // Create (4) or Destroy (6) row entry
    static const std::string  RSU_IFM_STATUS = "1.0.15628.4.1.5.1.7.0";

    // Specifies the current mode of operation of the RSU and provides capability to transition the device into a new mode, e.g. from the current mode to off, etc
    static const std::string  RSU_MODE = "1.0.15628.4.1.99.0";

    /*
    SYNTAX  INTEGER {
        bothOp (0), --both Continuous and Alternating modes are operational
        altOp (1),  --Alternating mode is operational,
                    --Continuous mode is not operational
        contOp (2), --Continuous mode is operational,
                    --Alternating mode is not operational
        noneOp (3)  --neither Continuous nor Alternating mode is operational
    */
    static const std::string  RSU_CHAN_STATUS = "1.0.15628.4.1.19.1.0";
}