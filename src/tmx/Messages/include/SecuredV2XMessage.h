/*
 * SecuredV2XMessage.h
 */
#ifndef INCLUDE__SecuredV2XMessage_H_
#define INCLUDE__SecuredV2XMessage_H_
#include <tmx/messages/message.hpp>
#include <memory>
#include <vector>
#include <cstdint>
#include "MessageTypes.h"
namespace tmx {
namespace messages {

class DsrcMetadata : public tmx::message
{
public:
    DsrcMetadata() {}
    DsrcMetadata(const tmx::message_container_type &contents): tmx::message(contents) {}

    /// DSRC PSID.
    std_attribute(this->msg, int, psid, 0, )
    /// DSRC channel.
    std_attribute(this->msg, int, channel, 0, )
};

class SecuredV2XMessage : public tmx::message
{
public:
SecuredV2XMessage() {}
SecuredV2XMessage(const tmx::message_container_type &contents): tmx::message(contents) {}
    /// Full SPDU bytes 
std_attribute(this->msg, std::string, spdu_data, "", )
    /// Packet UUID 
std_attribute(this->msg, std::string, uuid, "", )
    /// J2735 payload type, e.g. "BSM", "PSM", "MAP".
std_attribute(this->msg, std::string, MessageType, "", )
    /// timestamp
std_attribute(this->msg, int64_t, timestamp_ms, 0, )
    /// DSRC metadata: PSID and channel.
std_attribute(this->msg, DsrcMetadata, dsrc_metadata, DsrcMetadata(), )

    // Smart-pointer accessors for spdu_data
void set_spdu_data_bytes(std::shared_ptr<std::vector<uint8_t>> bytes) {
set_spdu_data(std::string(bytes->begin(), bytes->end()));
    }
std::shared_ptr<std::vector<uint8_t>> get_spdu_data_bytes() {
std::string s = get_spdu_data();
return std::make_shared<std::vector<uint8_t>>(s.begin(), s.end());
    }
    // Smart-pointer accessors for uuid
void set_uuid_bytes(std::shared_ptr<std::vector<uint8_t>> bytes) {
set_uuid(std::string(bytes->begin(), bytes->end()));
    }
std::shared_ptr<std::vector<uint8_t>> get_uuid_bytes() {
std::string s = get_uuid();
return std::make_shared<std::vector<uint8_t>>(s.begin(), s.end());
    }
};
} 
#endif /* INCLUDE__SecuredV2XMessage_H_ */