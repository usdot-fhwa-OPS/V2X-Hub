#pragma once
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <tmx/TmxException.hpp>
#include <jsoncpp/json/json.h>


namespace ImmediateForward
{
    static constexpr const char *ImmediateForwardConfigKey = "ImmediateForwardConfigurations";
    static constexpr const char *RSUSpecKey = "rsuSpec";
    static constexpr const char *AddressKey = "address";
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
        NTCIP_1218 = 2
    };

    struct Message {
        std::string tmxType;
        std::string sendType;
        std::string psid;
        int channel;
    };

    struct ImfConfiguration{

        RSU_SPEC spec;
        std::string address;
        std::vector<Message> messages;
        TxMode mode;
        bool signMessage;
        std::optional<bool> enableHsm;
        std::optional<std::string> hsmUrl;
        
    };
    const static std::unordered_map<std::string, TxMode> stringToTxModeMap = {
        { "CONT", TxMode::CONT},
        { "ALT", TxMode::ALT}    
    };



    const static std::unordered_map<std::string, RSU_SPEC> stringToRSUSpecMap = {
        { "RSU4.1", RSU_SPEC::RSU_4_1},
        { "NTCIP1218", RSU_SPEC::NTCIP_1218}    
    };

    std::vector<ImfConfiguration> parseImmediateForwardConfiguration(const std::string &config);

    Message parseMessage(const Json::Value &message);

    ImfConfiguration parseImfConfiguration( const Json::Value &imfConfig);

    std::string txModeToString(const TxMode &mode);

    TxMode stringToTxMode(const std::string &mode);

    std::string rsuSpecToString(const RSU_SPEC &spec);

    RSU_SPEC stringToRSUSpec( const std::string &spec);
}