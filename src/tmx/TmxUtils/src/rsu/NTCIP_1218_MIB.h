#pragma once

#include <string>

namespace tmx::utils::rsu::mib::ntcip1218
{
    /**
     * @brief This header file contains a subset of RSU-specific NTCIP 1218 MIB OIDs from https://www.ntcip.org/file/2021/01/NTCIP-1218v0138-RSU-toUSDOT-20200905.pdf
     */

    // RSU Identifier. Contains the ID given to this RSU.
    static const std::string rsuIDOid = "1.3.6.1.4.1.1206.4.2.18.13.4.0";

    // RSU MIB Version. Contains the version of this MIB supported by this RSU, e.g. rsuMIB 4.1 rev201812060000Z
    static const std::string rsuMibVersionOid = "1.3.6.1.4.1.1206.4.2.18.13.1.0";

    // RSU Firmware Version. Contains the version of firmware running on this RSU.
    static const std::string rsuFirmwareVersionOid = "1.3.6.1.4.1.1206.4.2.18.13.2.0";

    // RSU Radio Description. Name of the radio that the configuration relates to.
    static const std::string rsuRadioDescOid = "1.3.6.1.4.1.1206.4.2.18.1.2.1.2.0";

    // RSU Radio type indicates the type of V2X Radio. pC5 is cellular V2X
    static const std::string rsuRadioTypeOid = "1.3.6.1.4.1.1206.4.2.18.1.2.1.4.0";

    // GNSS Data Output. Contains NMEA 0183 GPGGA or GNGGA output string including the $ starting character and the ending <CR><LF>.
    static const std::string rsuGnssOutputStringOid = "1.3.6.1.4.1.1206.4.2.18.6.5.0";

    /* RSU Radio Status
    SYNTAX  INTEGER {
        bothOp (0), --both Continuous and Alternating modes are operational
        altOp (1),  --Alternating mode is operational,
                    --Continuous mode is not operational
        contOp (2), --Continuous mode is operational,
                    --Alternating mode is not operational
        noneOp (3)  --neither Continuous nor Alternating mode is operational
    */
    static const std::string rsuChanStatusOid = "1.3.6.1.4.1.1206.4.2.18.16.1.0";

    // RSU Mode. Commands the current mode of operation of the RSU and provides capability to transition the device into a new mode, e.g. from the operate mode to standby mode, etc.
    static const std::string rsuModeOid = "1.3.6.1.4.1.1206.4.2.18.16.2.0";
    /**
     * The maximum number of Immediate Forward messages this Roadside Unit supports. This object indicates the
     * maximum rows which appears in the rsuIFMStatusTable object
     */
    static const std::string maxRsuIFMs = "1.3.6.1.4.1.1206.4.2.18.4.1.0";

    // Forward Message PSID. The Provider Service Identifier (PSID) for the Immediate Forward Message. The current PSID assignments can be found at https://standards.ieee.org/products-services/regauth/psid/public.html.
    static const std::string rsuIFMPsidOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.2";

    // Forward Message Transmission Channel. The transmission channel the Immediate Forward Message is to be transmitted. For DSRC radios in the United States, the transmission channel is from 172 to 184, as defined by IEEE 802.11.
    static const std::string rsuIFMTxChannelOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.3";

    // Forward Message Enable. Set this bit to enable transmission of the message. 1 is to enable transmission.
    static const std::string rsuIFMEnableOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.4";

    // Forward Message Status. Create (4) or Destroy (6) row entry
    static const std::string rsuIFMStatusOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.5";
        // Forward Message Index. This index shall not exceed maxRsuIFMs.
    static const std::string rsuIFMIndexOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.1";
    
    // Forward Message Priority. Priority assigned to the Immediate Forward message. Priority values defined by IEEE 1609.3-2016 for DSRC radios.
    static const std::string rsuIFMPriorityOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.6";

    /* Forward Message Options. 
    A bit-mapped value as defined below for configuring the message.
        Bit 0 0=Bypass1609.2, 1=Process1609.2
        Bit 1 0=Secure, 1=Unsecure
        Bit 2 0=ContXmit, 1=NoXmitShortTermXceeded
        Bit 3 0=ContXmit, 1=NoXmitLongTermXceeded
    */
    static const std::string rsuIFMOptionsOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.7";

    // Forward Message Payload. Payload of Immediate Forward message.
    static const std::string rsuIFMPayloadOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.8";   


}
