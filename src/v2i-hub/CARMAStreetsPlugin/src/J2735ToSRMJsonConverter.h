#include "jsoncpp/json/json.h"
#include <tmx/j2735_messages/SignalRequestMessage.hpp>

namespace CARMAStreetsPlugin
{
    class J2735ToSRMJsonConverter
    {
    private:
        const std::string MsgType = "SRM";

    public:
        J2735ToSRMJsonConverter();
        void toSRMJson(Json::Value &json, tmx::messages::SrmMessage *srm);
        ~J2735ToSRMJsonConverter();
    };
}