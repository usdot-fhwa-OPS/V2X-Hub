#pragma once

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

    /** Maximum nesting depth allowed when unwrapping a 1609.2 SPDU.*/
    constexpr int MAX_SPDU_DEPTH = 20;

    /**
     * @brief Unwraps a 1609.2 SPDU to extract the unsecured payload bytes, PSID, and whether the PSID was set.
     * Returns the size of the extracted payload, or -1 on error.
     * 
     * @param d Ieee1609Dot2Data_t pointer to the SPDU data to unwrap.
     * @param payloadOut the byte_stream to store the extracted unsecured payload bytes.
     * @param psidOut stores the extracted PSID if found.
     * @param psidSet boolean flag indicating whether the PSID was set.
     * @param depth the current recursion depth (used to prevent excessive nesting).
     * @return int (size of extracted payload, or -1 on error)
     */
    inline int unwrapSpdu(const Ieee1609Dot2Data_t *d, std::vector<uint8_t>& payloadOut,
                        uint32_t& psidOut, bool& psidSet, int depth)   // no default here
    {
        if (!d || depth > MAX_SPDU_DEPTH) 
        {
            return -1;
        }
        if (d->protocolVersion != 3)      
        {
            return -1;
        }
        if (!d->content)                  
        {
            return -1;
        }

        switch (d->content->present) {
            case Ieee1609Dot2Content_PR_unsecuredData: {
                const Opaque_t& op = d->content->choice.unsecuredData;
                if (!op.buf || op.size <= 0) 
                {
                    return -1;
                }
                payloadOut.assign(op.buf, op.buf + op.size);
                return static_cast<int>(op.size);
            }
            case Ieee1609Dot2Content_PR_signedData: {
                const SignedData_t* sd = d->content->choice.signedData;
                if (!sd || !sd->tbsData) 
                {
                    return -1;
                }
                if (!psidSet) {
                    psidOut = static_cast<uint32_t>(sd->tbsData->headerInfo.psid);
                    psidSet = true;
                }
                if (!sd->tbsData->payload) 
                {
                    return -1;
                }
                const Ieee1609Dot2Data_t* next = sd->tbsData->payload->data;  // OPTIONAL
                if (!next) 
                {
                    return false;
                }
                return unwrapSpdu(next, payloadOut, psidOut, psidSet, depth + 1);
            }
            default:
                return -1;
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
     * @brief Builds a RawSpdu message from the given corresponding field data.
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
        spduMsg.set_uuid(uuid);
        spduMsg.set_timestampMs(rxTime);
        spduMsg.set_psid(psid);
        spduMsg.set_messageType("Missing ID"); // TODO: Placeholder until we update this to include string message ID from payload

        return spduMsg;

    }

} // namespace MessageReceiver

