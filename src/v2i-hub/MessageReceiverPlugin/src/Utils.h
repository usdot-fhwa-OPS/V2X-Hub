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
constexpr int MAX_SPDU_DEPTH = 4;




inline bool unwrapSpdu(const Ieee1609Dot2Data_t *d, std::vector<uint8_t>& payloadOut,
                       uint32_t& psidOut, bool& psidSet, int depth)   // no default here
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

inline std::shared_ptr<Ieee1609Dot2Data_t> decodeSpdu(const tmx::byte_stream& data, int len)
{
    Ieee1609Dot2Data_t* decodedPtr = nullptr;
    
    asn_dec_rval_t rv = oer_decode(0, &asn_DEF_Ieee1609Dot2Data,
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

inline tmx::messages::RawSpdu buildRawSpdu(uint psid, const tmx::byte_stream &bytes,  unsigned long rxTime, const boost::uuids::uuid& uuid)
{
    tmx::messages::RawSpdu spduMsg;
    spduMsg.set_spduData(bytes);
    spduMsg.set_uuid(tmx::byte_stream(uuid.begin(), uuid.end()));
    spduMsg.set_timestampMs(rxTime);
    spduMsg.set_psid(psid);
    spduMsg.set_messageType("Missing ID"); // TODO: Placeholder until we update this to include string message ID from payload

    return spduMsg;

}

} // namespace MessageReceiver

