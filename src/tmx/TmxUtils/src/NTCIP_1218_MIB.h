#pragma once
namespace tmx::utils::ntcip1218::mib::oid
{
    /**
     * @brief This header file contains a subset of RSU-specific NTCIP 1218 MIB OIDs from https://www.ntcip.org/file/2021/01/NTCIP-1218v0138-RSU-toUSDOT-20200905.pdf
     */

    // RSU Identifier. Contains the ID given to this RSU.
    static constexpr const char *rsuIDOid = "1.3.6.1.4.1.1206.4.2.18.13.4";

    // RSU MIB Version. Contains the version of this MIB supported by this RSU, e.g. rsuMIB 4.1 rev201812060000Z
    static constexpr const char *rsuMibVersionOid = "1.3.6.1.4.1.1206.4.2.18.13.1";

    // RSU Firmware Version. Contains the version of firmware running on this RSU.
    static constexpr const char *rsuFirmwareVersionOid = "1.3.6.1.4.1.1206.4.2.18.13.2";

    // RSU Radio Description. Name of the radio that the configuration relates to.
    static constexpr const char *rsuRadioDescOid = "1.3.6.1.4.1.1206.4.2.18.1.2.1.2";

    // GNSS Data Output. Contains NMEA 0183 GPGGA or GNGGA output string including the $ starting character and the ending <CR><LF>.
    static constexpr const char *rsuGnssOutputStringOid = "1.3.6.1.4.1.1206.4.2.18.6.5";

    // Forward Message Index. This index shall not exceed maxRsuIFMs.
    static constexpr const char *rsuIFMIndexOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.1";

    // Forward Message PSID. The Provider Service Identifier (PSID) for the Immediate Forward Message. The current PSID assignments can be found at https://standards.ieee.org/products-services/regauth/psid/public.html.
    static constexpr const char *rsuIFMPsidOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.2";

    // Forward Message Transmission Channel. The transmission channel the Immediate Forward Message is to be transmitted. For DSRC radios in the United States, the transmission channel is from 172 to 184, as defined by IEEE 802.11.
    static constexpr const char *rsuIFMTxChannelOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.3";

    // Forward Message Enable. Set this bit to enable transmission of the message. 1 is to enable transmission.
    static constexpr const char *rsuIFMEnableOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.4";

    // Forward Message Status. Create (4) or Destroy (6) row entry
    static constexpr const char *rsuIFMStatusOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.5";
    
    // Forward Message Priority. Priority assigned to the Immediate Forward message. Priority values defined by IEEE 1609.3-2016 for DSRC radios.
    static constexpr const char *rsuIFMPriorityOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.6";

    /* Forward Message Options. 
    A bit-mapped value as defined below for configuring the message.
        Bit 0 0=Bypass1609.2, 1=Process1609.2
        Bit 1 0=Secure, 1=Unsecure
        Bit 2 0=ContXmit, 1=NoXmitShortTermXceeded
        Bit 3 0=ContXmit, 1=NoXmitLongTermXceeded
    */
    static constexpr const char *rsuIFMOptionsOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.7";

    // Forward Message Payload. Payload of Immediate Forward message.
    static constexpr const char *rsuIFMPayloadOid = "1.3.6.1.4.1.1206.4.2.18.4.2.1.8";   


    /* RSU Radio Status
    SYNTAX  INTEGER {
        bothOp (0), --both Continuous and Alternating modes are operational
        altOp (1),  --Alternating mode is operational,
                    --Continuous mode is not operational
        contOp (2), --Continuous mode is operational,
                    --Alternating mode is not operational
        noneOp (3)  --neither Continuous nor Alternating mode is operational
    */
    static constexpr const char *rsuChanStatusOid = "1.3.6.1.4.1.1206.4.2.18.16.1";

    // RSU Mode. Commands the current mode of operation of the RSU and provides capability to transition the device into a new mode, e.g. from the operate mode to standby mode, etc.
    static constexpr const char *rsuModeOid = "1.3.6.1.4.1.1.1206.4.2.18.16.2";
}
