/*
 * RawSpdu.h
 */
#ifndef INCLUDE__RawSpdu _H_
#define INCLUDE__RawSpdu _H_
#include <tmx/messages/message.hpp>
#include <tmx/messages/byte_stream.hpp>
#include <memory>
#include <vector>
#include <cstdint>
#include "MessageTypes.h"
namespace tmx
{
    namespace messages
    {
        /**
         * RawSpdu - TMX message wrapper carrying a raw, encoded SPDU.
         *
         * Holds an already-encoded 1609.2-secured J2735 message, such as BSM, PSM,
         * MAP, SPaT, etc., to move raw V2X payloads across the TMX bus.
         *
         * Fields (none are set automatically; the caller must populate them before publishing):
         *   - spdu_data:    Raw SPDU bytes (stored as a std::vector<uint8_t>).
         *                   Use set_spdu_data()/get_spdu_data(), or the byte-vector
         *                   interface via set_spdu_data_bytes()/get_spdu_data_bytes().
         *   - uuid:         Unique identifier for this packet (stored as a
         *                   std::vector<uint8_t>), used for correlation and tracing.
         *                   Use set_uuid()/get_uuid().
         *   - MessageType:  J2735 type label, e.g., "BSM", "PSM", or "MAP".
         *   - timestamp_ms: Epoch timestamp in milliseconds, received or generated time
         *                   depending on the producer. Defaults to 0 if unset.
         *   - psid:         1609.2 PSID (Protocol Service Identifier),specified as an integer.
         *   - channel:      The channel on which the message should be  transmitted,specified as an integer. 
         */
        class RawSpdu : public tmx::message
        {
        public:
            RawSpdu() {}
            #if __cplusplus > 199711L
                /// Message type for routing this message through TMX core.
                static constexpr const char* MessageType = MSGTYPE_DECODED_STRING;

                /// Message sub type for routing this message through TMX core.
                static constexpr const char* MessageSubType = MSGSUBTYPE_BASIC_STRING;
            #endif
            RawSpdu(const tmx::message_container_type &contents) : tmx::message(contents) {}

                // Full SPDU bytes
                std_attribute(this->msg, tmx::byte_stream, spdu_data, tmx::byte_stream(), );
                // Packet UUID
                std_attribute(this->msg, tmx::byte_stream, uuid, tmx::byte_stream(), );

                // J2735 payload type, e.g. "BSM", "PSM", "MAP".
                std_attribute(this->msg, std::string, msg_type, "", );
                // timestamp
                std_attribute(this->msg, int64_t, timestamp_ms, 0, );
                /// 1609Dot2 PSID.
                std_attribute(this->msg, int, psid, 0, );
                ///  channel Number
                std_attribute(this->msg, int, channel, 0, );
                // Set the SPDU data as a vector of bytes
                void set_spdu_data_bytes(std::shared_ptr<std::vector<uint8_t>> bytes)
                {
                    set_spdu_data(tmx::byte_stream(bytes->begin(), bytes->end()));
                }
            std::shared_ptr<std::vector<uint8_t>> get_spdu_data_bytes()
            {
                tmx::byte_stream bs = get_spdu_data();
                return std::make_shared<std::vector<uint8_t>>(bs.begin(), bs.end());
            }
        };
    }
}
#endif /* INCLUDE__RawSpdu _H_ */