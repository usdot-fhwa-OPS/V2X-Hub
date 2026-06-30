/*
 * CTI4501ValidationMessage.h
 *
 * TMX message type for CTI 4501 intersection validation events.
 */
#ifndef INCLUDE_CTI4501VALIDATIONMESSAGE_H_
#define INCLUDE_CTI4501VALIDATIONMESSAGE_H_


#include <tmx/messages/message.hpp>
#include "MessageTypes.h"

namespace tmx::messages {

    struct ProcessingTimePeriod {
        int64_t beginTimestamp = 0;
        int64_t endTimestamp = 0;

        ProcessingTimePeriod() = default;
        ProcessingTimePeriod(int64_t begin, int64_t end)
            : beginTimestamp(begin), endTimestamp(end) {}

        static message_tree_type to_tree(const ProcessingTimePeriod &period) {
            message_tree_type tree;
            tree.put("beginTimestamp", period.beginTimestamp);
            tree.put("endTimestamp", period.endTimestamp);
            return tree;
        }

        static ProcessingTimePeriod from_tree(const message_tree_type &tree) {
            ProcessingTimePeriod period;
            period.beginTimestamp = tree.get<int64_t>("beginTimestamp");
            period.endTimestamp = tree.get<int64_t>("endTimestamp");
            return period;
        }
    };



    class CTI4501ValidationMessage : public tmx::message
    {
        public:
            CTI4501ValidationMessage() = default;
            explicit CTI4501ValidationMessage(const tmx::message_container_type &contents) : tmx::message(contents) {}
            ~CTI4501ValidationMessage() override{};

            static constexpr const char *MessageType = MSGTYPE_APPLICATION_STRING;
            static constexpr const char *MessageSubType = "CTI4501ValidationEvent";

            // Time when event was generated
            std_attribute(this->msg, int64_t, eventGeneratedAt, 0, )

            // Event type identifier (e.g. "SpatMinimumData", "MapMinimumData", "MapBroadcastRate", "SpatBroadcastRate"
            // "MapMessageCountProgressionEvent", "SpatMessageCountProgressionEvent", etc.)
            std_attribute(this->msg, std::string, eventType, "", )

            // Intersection ID from the message, or -1 if unavailable
            std_attribute(this->msg, int, intersectionID, -1, )

            // Road regulator ID from the message, or -1 if unavailable
            std_attribute(this->msg, int, roadRegulatorID, -1, )

            // RSU Identifier
            std_attribute(this->msg, std::string, source, "", )

            // Time period
            object_attribute(ProcessingTimePeriod, timePeriod)

            // Missing CTI 4501 required fields
            array_attribute(std::string, missingDataElements)

            // Timestamp A
            std_attribute(this->msg, std::string, timestampA, "", )
            std_attribute(this->msg, std::string, timestampB, "", )

            // Message Count A
            std_attribute(this->msg, int64_t, messageCountA, 0, )

            // Message Count B
            std_attribute(this->msg, int64_t, messageCountB, 0, )

            // Kafka topic whose rate was measured (BroadcastRate)
            std_attribute(this->msg, std::string, topicName, "", )

            // Number of messages observed in the time period (BroadcastRate)
            std_attribute(this->msg, int, numberOfMessages, 0, )

            // "SPAT" or "MAP" (MessageCountProgression)
            std_attribute(this->msg, std::string, messageType, "", )
    };

} /* namespace tmx::messages */

#endif /* INCLUDE_CTI4501VALIDATIONMESSAGE_H_ */
