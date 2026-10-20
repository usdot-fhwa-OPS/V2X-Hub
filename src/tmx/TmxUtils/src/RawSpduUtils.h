/*
 * RawSpduUtils.h
 *
 * Helpers for decoding IEEE 1609.2 SPDUs and building the RawSpdu message that
 * carries them across TMX. These live next to RawSpdu.h so that every plugin
 * handling secured V2X traffic shares one implementation.
 *
 * These functions are inline, so tmxmessages remains a header-only install and
 * gains no link dependency. Consumers that include this header must link
 * ::stol-1609dot2-2022 themselves and add /opt/carma/include/stol-1609dot2-2022
 * to their include path.
 */

#ifndef TMX_UTILS_RAWSPDUUTILS_H_
#define TMX_UTILS_RAWSPDUUTILS_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

#include <RawSpdu.h>
#include "PluginLog.h"
#include "TmxLog.h"

#include <stol-1609dot2-2022/Ieee1609Dot2Data.h>
#include <stol-1609dot2-2022/Ieee1609Dot2Content.h>

#include <tmx/TmxException.hpp>
#include <tmx/messages/byte_stream.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>

#include <boost/uuid/uuid.hpp>

namespace tmx::utils {

    std::mutex factoryMutex_;
    static tmx::messages::J2735MessageFactory spduUtilJ2735Factory_;

    /** Maximum nesting depth allowed when unwrapping a 1609.2 SPDU. */
    constexpr int MAX_SPDU_DEPTH = 20;

    /**
     * @brief Unwraps a 1609.2 SPDU to extract the unsecured payload bytes, PSID, and whether the PSID was set.
     * Returns boolean indicating success or failure.
     *
     * @param d Ieee1609Dot2Data_t pointer to the SPDU data to unwrap.
     * @param payloadOut the byte_stream to store the extracted unsecured payload bytes.
     * @param psidOut stores the extracted PSID if found.
     * @param psidSet boolean flag indicating whether the PSID was set.
     * @param depth the current recursion depth (used to prevent excessive nesting).
     * @param psidOnly if true, only extracts the PSID without extracting the payload.
     * @return bool
     */
    inline bool unwrapSpdu(const Ieee1609Dot2Data_t *d, std::vector<uint8_t>& payloadOut,
                        uint32_t& psidOut, bool& psidSet, int depth, bool psidOnly=false)   // no default here
    {
        if (!d || depth > MAX_SPDU_DEPTH)
        {
            return false;
        }
        if (d->protocolVersion != 3)
        {
            return false;
        }
        if (!d->content)
        {
            return false;
        }

        switch (d->content->present) {
            case Ieee1609Dot2Content_PR_unsecuredData: {
                const Opaque_t& op = d->content->choice.unsecuredData;
                if (!op.buf || op.size <= 0)
                {
                    return false;
                }
                payloadOut.assign(op.buf, op.buf + op.size);
                return true;
            }
            case Ieee1609Dot2Content_PR_signedData: {
                const SignedData_t* sd = d->content->choice.signedData;
                if (!sd || !sd->tbsData)
                {
                    return false;
                }
                if (!psidSet) {
                    psidOut = static_cast<uint32_t>(sd->tbsData->headerInfo.psid);
                    psidSet = true;
                    if (psidOnly) {
                        return true;  // If only PSID is needed, return early
                    }
                }
                if (!sd->tbsData->payload)
                {
                    return false;
                }
                const Ieee1609Dot2Data_t* next = sd->tbsData->payload->data;  // OPTIONAL
                if (!next)
                {
                    return false;
                }
                return unwrapSpdu(next, payloadOut, psidOut, psidSet, depth + 1);
            }
            default:
                return false;
        }
    }

    /**
     * @brief Decodes a 1609.2 SPDU from a byte stream into an Ieee1609Dot2Data_t structure.
     *
     * @param data the byte stream buffer to decode from.
     * @param len the length of the byte stream to use.
     * @return std::shared_ptr<Ieee1609Dot2Data_t>
     */
    inline std::shared_ptr<Ieee1609Dot2Data_t> decodeSpdu(const tmx::byte_stream& data, int len)
    {
        Ieee1609Dot2Data_t* decodedPtr = nullptr;

        asn_dec_rval_t rv = oer_decode(nullptr, &asn_DEF_Ieee1609Dot2Data,
                                    (void**)&decodedPtr, data.data(), len);

        if (rv.code != RC_OK || !decodedPtr) {
            ASN_STRUCT_FREE(asn_DEF_Ieee1609Dot2Data, decodedPtr);  // Free if allocated
            throw tmx::TmxException("Failed to decode SPDU: " + std::to_string(rv.consumed) + " bytes consumed of "
                + std::to_string(len) + ".");
        }
        std::shared_ptr<Ieee1609Dot2Data_t> decoded(
            decodedPtr,
            [](Ieee1609Dot2Data_t* p){
                ASN_STRUCT_FREE(asn_DEF_Ieee1609Dot2Data, p);
            }
        );

        return decoded;
    }

    /**
     * @brief Resolve the J2735 subtype label (e.g. "BSM", "PSM-P") for an encoded payload.
     *
     * @param payload the unsecured J2735 payload bytes.
     * @return the subtype label, or tmx::messages::api::MSGSUBTYPE_UNKNOWN_STRING if unidentifiable.
     */
    inline std::string getJ2735SubType(const tmx::byte_stream &payload)
    {   
        if (payload.empty())
        {
            return UNKNOWN_SUBTYPE;
        }

        try
        {   
            tmx::byte_stream payloadCopy(payload);
            std::lock_guard<std::mutex> guard(factoryMutex_); // ensure thread-safe access to the factory
            auto id = spduUtilJ2735Factory_.GetMessageId(payloadCopy);

            if(id >= 0){
                auto msgType = spduUtilJ2735Factory_.MessageType(id);
                if(msgType){
                    return std::string(msgType);
                }
                else {
                    return UNKNOWN_SUBTYPE;
                }
            }
            else {
                return UNKNOWN_SUBTYPE;
            }

        }
        catch (const std::exception &e)
        {   
            // log error and return unknown type in case of exception
            FILE_LOG(logERROR) << "Error determining J2735 subtype from payload bytes : " << e.what();
            return UNKNOWN_SUBTYPE;
        }
        
    }

    /**
     * @brief Builds a RawSpdu message from the given corresponding field data.
     *
     * The message type is derived from the unsecured payload via the J2735
     * factory rather than being hard-coded.
     *
     * @param psid
     * @param fullByteData
     * @param msgByteData
     * @param rxTime
     * @param uuid
     * @return tmx::messages::RawSpdu
     */
    inline tmx::messages::RawSpdu buildRawSpdu(uint psid, const tmx::byte_stream &fullByteData, const tmx::byte_stream &msgByteData, unsigned long rxTime, const boost::uuids::uuid& uuid)
    {
        tmx::messages::RawSpdu spduMsg;
        spduMsg.set_fullByteData(fullByteData);
        spduMsg.set_msgByteData(msgByteData);
        spduMsg.set_uuid(tmx::byte_stream(uuid.begin(), uuid.end()));
        spduMsg.set_timestampMs(rxTime);
        spduMsg.set_psid(psid);
        spduMsg.set_messageType(getJ2735SubType(msgByteData));

        return spduMsg;
    }

    /**
     * @brief Assigns a PSID to a message type.
     * Using constexpr to allow compile-time evaluation for known message types, avoiding overhead of using hashmap.
     *
     * @param msgType The message type.
     * @return The assigned PSID.
     */
    constexpr tmx::messages::api::msgPSID assignMsgPSID(std::string_view msgType)
    {
        using namespace tmx::messages::api;
        for (const auto &entry : MSGTYPE_PSID_PAIRS)
        {
            if (msgType == entry.first)
                return entry.second;
        }
        return msgPSID::None_PSID;
    }

} // namespace tmx::utils

#endif /* TMX_UTILS_RAWSPDUUTILS_H_ */
