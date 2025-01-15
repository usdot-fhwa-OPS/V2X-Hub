#pragma once
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <tmx/TmxException.hpp>
#include <jsoncpp/json/json.h>


namespace ImmediateForward
{
    static constexpr const char *NameKey = "name";
    static constexpr const char *RSUSpecKey = "rsuSpec";
    static constexpr const char *AddressKey = "address";
    static constexpr const char *PortKey = "port";
    static constexpr const char *MessagesKey = "messages";
    static constexpr const char *TxModeKey = "txMode";
    static constexpr const char *SignKey = "signMessages";
    static constexpr const char *EnableHSMKey = "enableHsm";
    static constexpr const char *HSMURLKey = "hsmUrl";
    static constexpr const char *TmxTypeKey = "tmxType";
    static constexpr const char *SendTypeKey = "sendType";
    static constexpr const char *PSIDKey = "psid";
    static constexpr const char *ChannelKey = "channel";


    enum class TxMode
    {
        CONT = 0,
        ALT = 1
    };

    enum class RSU_SPEC {
        RSU_4_1 = 0,
        NTCIP_1218 = 1
    };
    /**
     * Message configuration
     */
    struct Message {
        std::string tmxType;
        std::string sendType;
        std::string psid;
        std::optional<int> channel;
    };
    /**
     * Immediate forward configuration for an RSU connection
     */
    struct ImfConfiguration{

        std::string name;
        RSU_SPEC spec;
        std::string address;
        unsigned int port;
        std::vector<Message> messages;
        TxMode mode;
        bool signMessage;
        std::optional<bool> enableHsm;
        std::optional<std::string> hsmUrl;
        
    };
    /** 
     * Map to convert between string TxMode and enumeration
     */
    const static std::unordered_map<std::string, TxMode> stringToTxModeMap = {
        { "CONT", TxMode::CONT},
        { "ALT", TxMode::ALT}    
    };

    /**
     * Map to conver between string RSU spec and enumeration
     */
    const static std::unordered_map<std::string, RSU_SPEC> stringToRSUSpecMap = {
        { "RSU4.1", RSU_SPEC::RSU_4_1},
        { "NTCIP1218", RSU_SPEC::NTCIP_1218}    
    };

    /**
     * Function to parse Immediate Forward Configurations for RSU connections and return vector of ImfConfigurations
     */
    std::vector<ImfConfiguration> parseImmediateForwardConfiguration(const std::string &config);

    /**
     * Helper function to parse Message JSON
     */
    Message parseMessage(const Json::Value &message);

    /**
     * Helper function to parse ImfConfiguration JSON
     */
    ImfConfiguration parseImfConfiguration( const Json::Value &imfConfig);

    /**
     * Helper function to convert TxMode enumeration to string
     */
    std::string txModeToString(const TxMode &mode);
    
    /**
     * Helper function to convert string TxMode to enumeration
     */
    TxMode stringToTxMode(const std::string &mode);

    /**
     * Helper function to convert RSU Spec enumeration to string
     */
    std::string rsuSpecToString(const RSU_SPEC &spec);

    /**
     * Helper function to convert string to RSUS Spec enumeration
     */
    RSU_SPEC stringToRSUSpec( const std::string &spec);
}