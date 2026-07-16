#ifndef SRC_MESSAGERECEIVER_UTILS_H_
#define SRC_MESSAGERECEIVER_UTILS_H_

#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <RawSpdu.h>
#include <stol-1609dot2-2022/Ieee1609Dot2Data.h>
#include <stol-1609dot2-2022/Ieee1609Dot2Content.h>

#include <PluginClient.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace MessageReceiver {

/** Maximum offset (in hex characters) within which a J2735 message ID must appear. */
constexpr int IDCHECKLIMIT   = 60;
/** Maximum nesting depth allowed when unwrapping a 1609.2 SPDU.*/
constexpr int MAX_SPDU_DEPTH = 4;

/**
 * Identifies the J2735 message type of an already-unwrapped payload.
 *
 * @param messageIds Configured message IDs to look for, as hex strings (e.g. "0013").
 * @param payload    Raw J2735 MessageFrame bytes.
 * @return The numeric DSRCmsgID (e.g. 19 for SPaT), or -1 if no configured ID was found.
 */
inline int64_t identifyJ2735Type(const std::vector<std::string>& messageIds,  const std::vector<uint8_t>& payload);

/**
 * Locates a configured J2735 message ID within a hex-encoded message and decodes
 * the length of the frame that follows it.
 *
 * @param messageIds Configured message IDs to look for, as hex strings.
 * @param hex        Hex-encoded message to search.
 * @param idloc      [out] Character offset of the message ID within hex.
 * @param hexLen     [out] Total length of the J2735 frame, in hex characters.
 * @param dsrcMsgId  [out] Numeric value of the matched message ID.
 * @return true if a configured ID was found within IDCHECKLIMIT, false otherwise.
 */
inline bool findMessageId(const std::vector<std::string>& messageIds, const std::string& hex, size_t& idloc, int& hexLen, long& dsrcMsgId);

/**
 * Recursively unwraps a decoded 1609.2 SPDU to the innermost unsecured J2735 payload,
 * descending through any nested signedData layers.
 *
 * @param d          Decoded Ieee1609Dot2Data to unwrap.
 * @param payloadOut [out] Receives the unsecured J2735 payload bytes on success.
 * @param psidOut    [out] PSID from the outermost signed layer; unchanged if none is present.
 * @param psidSet    [in,out] Tracks whether psidOut has been set; pass false initially.
 * @param depth      Current recursion depth; callers should omit this - since this is updated on recursive calls.
 * @return true if an unsecured payload was reached, false on encrypted content,
 *         a missing payload, or nesting beyond MAX_SPDU_DEPTH.
 */
inline bool unwrapSpdu(const Ieee1609Dot2Data_t* d, std::vector<uint8_t>& payloadOut, uint32_t& psidOut, bool& psidSet, int depth = 0);


inline bool buildRawSpduMessage(const uint8_t* data, int len, uint64_t rxTime,
                                const std::vector<std::string>& messageIds,
                                const boost::uuids::uuid& uuid, tmx::messages::RawSpdu& out, std::vector<uint8_t>& payloadOut);
/**
 * Decodes a 1609.2 SPDU, extracts its inner J2735 payload, and populates a RawSpdu message.
 * The signature is not verified.
 *
 * @param data       Raw SPDU bytes as received.
 * @param len        Number of bytes in data.
 * @param rxTime     Reception timestamp, in milliseconds since the epoch.
 * @param messageIds Configured message IDs used to identify the inner payload.
 * @param uuid       Unique identifier to assign to this packet.
 * @param out        [out] RawSpdu message populated on success.
 * @param payloadOut [out] Receives the unwrapped J2735 payload bytes on success.
 * @return true if the SPDU decoded and unwrapped successfully, false otherwise.
 */
inline bool findMessageId(const std::vector<std::string>& messageIds, const std::string& hex,
                          size_t& idloc, int& hexLen, long& dsrcMsgId)
{
    for (const auto& id : messageIds)
    {
        size_t loc = hex.find(id);
        if (loc != std::string::npos && loc < IDCHECKLIMIT)
        {
            int mlen;
            if (hex[loc + 4] == '8')                       // length > 256, long form
            {
                std::string tmp = hex.substr(loc + 5, 3);
                mlen = (strtol(tmp.c_str(), nullptr, 16) + 4) * 2;
            }
            else                                           // short form
            {
                std::string tmp = hex.substr(loc + 4, 2);
                mlen = (strtol(tmp.c_str(), nullptr, 16) + 3) * 2;
            }
            idloc     = loc;
            hexLen    = mlen;
            dsrcMsgId = strtol(id.c_str(), nullptr, 16);
            return true;
        }
    }
    return false;
}

inline int64_t identifyJ2735Type(const std::vector<std::string>& messageIds,
                                 const std::vector<uint8_t>& payload)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    size_t n = std::min<size_t>(payload.size(), IDCHECKLIMIT / 2);
    for (size_t i = 0; i < n; ++i)
        ss << std::setw(2) << static_cast<unsigned>(payload[i]);

    size_t idloc; int mlen; long msgId;
    return findMessageId(messageIds, ss.str(), idloc, mlen, msgId) ? msgId : -1;
}

inline bool unwrapSpdu(const Ieee1609Dot2Data_t* d, std::vector<uint8_t>& payloadOut,
                       uint32_t& psidOut, bool& psidSet, int depth)   // no default here
{
    if (!d || depth > MAX_SPDU_DEPTH) return false;
    if (d->protocolVersion != 3)      return false;
    if (!d->content)                  return false;

    switch (d->content->present) {
        case Ieee1609Dot2Content_PR_unsecuredData: {
            const Opaque_t& op = d->content->choice.unsecuredData;
            if (!op.buf || op.size <= 0) return false;
            payloadOut.assign(op.buf, op.buf + op.size);
            return true;
        }
        case Ieee1609Dot2Content_PR_signedData: {
            const SignedData_t* sd = d->content->choice.signedData;
            if (!sd || !sd->tbsData) return false;
            if (!psidSet) {
                psidOut = static_cast<uint32_t>(sd->tbsData->headerInfo.psid);
                psidSet = true;
            }
            if (!sd->tbsData->payload) return false;
            const Ieee1609Dot2Data_t* next = sd->tbsData->payload->data;  // OPTIONAL
            if (!next) return false;
            return unwrapSpdu(next, payloadOut, psidOut, psidSet, depth + 1);
        }
        default:
            return false;
    }
}

inline bool buildRawSpduMessage(const uint8_t* data, int len, uint64_t rxTime,
                                const std::vector<std::string>& messageIds,
                                const boost::uuids::uuid& uuid,
                                tmx::messages::RawSpdu& out,
                                std::vector<uint8_t>& payloadOut)
{
    // 1. Decode SPDU
    Ieee1609Dot2Data_t* decodedPtr = nullptr;
    asn_dec_rval_t rv = oer_decode(nullptr, &asn_DEF_Ieee1609Dot2Data,
                                   (void**)&decodedPtr, data, len);

    auto del = [](Ieee1609Dot2Data_t* p){ ASN_STRUCT_FREE(asn_DEF_Ieee1609Dot2Data, p); };
    std::unique_ptr<Ieee1609Dot2Data_t, decltype(del)> decoded(decodedPtr, del);

    if (rv.code != RC_OK || !decoded) return false;

    // 2. Walk to the inner J2735 payload
    uint32_t psid = 0;
    bool psidSet = false;
    if (!unwrapSpdu(decoded.get(), payloadOut, psid, psidSet)) return false;

    // 3. Populate tmx message
    out.set_spduData(tmx::byte_stream(data, data + len));
    out.set_uuid(tmx::byte_stream(uuid.begin(), uuid.end()));
    out.set_timestampMs(rxTime);
    out.set_psid(psid);
    out.set_messageType(std::to_string(identifyJ2735Type(messageIds, payloadOut))); // Currently includes id num as string. This should be updated to map to the message type by name
    return true;
}

} // namespace MessageReceiver

#endif /* SRC_MESSAGERECEIVER_UTILS_H_ */