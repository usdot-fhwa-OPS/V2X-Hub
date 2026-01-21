#include <string>
#include <jsoncpp/json/json.h>

namespace TelematicBridge
{
    /**
     * @brief Topic message with name and selection status
     */
    class TopicMessage
    {
    private:
        std::string name;
        bool selected;
    public:
        TopicMessage() : name(""), selected(false) {}
        
        TopicMessage(const std::string& topicName, bool isSelected = false)
            : name(topicName), selected(isSelected) {}

        /**
         * @brief Convert TopicMessage to JSON object
         * @return Json::Value JSON representation
         */
        Json::Value toJson() const
        {
            Json::Value json;
            json["name"] = name;
            json["selected"] = selected;
            return json;
        }

        void setName(const std::string& topicName) { name = topicName; }
        void setSelected(bool isSelected) { selected = isSelected; }
        std::string getName() const { return name; }
        bool isSelected() const { return selected; }
    };
}