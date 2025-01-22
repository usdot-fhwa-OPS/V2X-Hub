#pragma once
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <tmx/TmxException.hpp>
#include <jsoncpp/json/json.h>
#include <rsu/RSUSpec.h>


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
    static constexpr const char *SNMPAuthKey = "snmpAuth";
    static constexpr const char *UserKey = "user";
    static constexpr const char *AuthProtocolKey = "authProtocol";
    static constexpr const char *PrivProtocolKey = "privacyProtocol";
    static constexpr const char *AuthPassPhraseKey = "authPassPhrase";
    static constexpr const char *PrivPassPhraseKey = "privacyPassPhrase";
    static constexpr const char *SecurityLevelKey = "securityLevel";
    static constexpr const char *CommunityKey = "community";



    enum class TxMode
    {
        CONT = 0,
        ALT = 1
    };

    enum class SecurityLevel {
        NO_AUTH_NO_PRIV = 0,
        AUTH_NO_PRIV =1,
        AUTH_PRIV
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

    struct SNMPAuth {
        std::string user;
        SecurityLevel securityLevel;
        std::string community;
        // Optional depending on security level
        std::optional<std::string> authProtocol;
        std::optional<std::string> privProtocol;
        std::optional<std::string> authPassPhrase;
        std::optional<std::string> privPassPhrase;
    };
    /**
     * Immediate forward configuration for an RSU connection
     */
    struct ImfConfiguration{

        std::string name;
        tmx::utils::rsu::RSU_SPEC spec;
        std::string address;
        unsigned int port;
        std::optional<SNMPAuth> snmpAuth;
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
     * Map to convert between string SecurityLevel and enumeration
     */
    const static std::unordered_map<std::string, SecurityLevel> stringToSecurityLevelMap = {
        {"NoAuthNoPriv", SecurityLevel::NO_AUTH_NO_PRIV},
        {"AuthNoPriv", SecurityLevel::AUTH_NO_PRIV},
        {"AuthPriv", SecurityLevel::AUTH_PRIV}
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
     * Helper function to parse SNMPAuth JSON
     */
    SNMPAuth parseSNMPAuth( const Json::Value &SNMPAuth) ;

    /**
     * Helper function to convert TxMode enumeration to string
     */
    std::string txModeToString(const TxMode &mode);
    
    /**
     * Helper function to convert string TxMode to enumeration
     */
    TxMode stringToTxMode(const std::string &mode);

    /**
     * Helper function to convert SecurityLevel enumeration to string
     */
    std::string securityLevelToString(const SecurityLevel &level);
    
    /**
     * Helper function to convert string SecurityLevel to enumeration
     */
    SecurityLevel stringToSecurityLevel(const std::string &level);
}