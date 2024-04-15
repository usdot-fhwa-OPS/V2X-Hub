#pragma once
#include "PluginLog.h"
#include <tmx/messages/TmxJ2735.hpp>
#include "TelematicBridgeException.h"
#include "jsoncpp/json/json.h"
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace tmx::utils;
using namespace std;
using namespace tmx::messages;
namespace pt = boost::property_tree;

namespace TelematicBridge
{
    using buffer_structure_t = struct buffer_structure
    {
        char *buffer;          // buffer array
        size_t buffer_size;    // this is really where we will write next.
        size_t allocated_size; // this is the total size of the buffer.
    };

    /**
     * @brief Convert hex string into a vector of bytes
     * @param string input hex string payload
     * @param vector<char*> byte buffer to be updated.
     * @return bool indicator whether conversion is successful.
     */
    bool HexToBytes(const string &hexPaylod, vector<char> &byteBuffer)
    {
        uint8_t d = 0;
        int i = 0;

        for (const char &c : hexPaylod)
        {
            if (c <= '9' && c >= '0')
            {
                d = c - '0';
            }
            else if (c <= 'F' && c >= 'A')
            {
                d = c - 55; // c - 'A' + 10
            }
            else if (c <= 'f' && c >= 'a')
            {
                d = c - 87; // c - 'a' + 10;
            }
            else
            {
                return false;
            }

            if (i % 2)
            {
                // low order nibble.
                byteBuffer.back() |= d;
            }
            else
            {
                // high order nibble.
                byteBuffer.push_back(d << 4);
            }
            ++i;
        }
        return true;
    }

    /**
     * @brief Decode J2735 message and populate the J2735 data frame
     * @param string input hex string payload
     * @param MessageFrame_t J2735 struct to be updated
     * @return bool indicator whether decoding successful
     */
    void DecodeJ2735Msg(const string &hexPaylod, MessageFrame_t *messageFrame)
    {
        /**
         * Decode J2735 message
         */
        ostringstream erroross;
        vector<char> byte_buffer;
        if (!HexToBytes(hexPaylod, byte_buffer))
        {
            throw TelematicBridgeException("Failed attempt to decode MessageFrame hex string: cannot convert to bytes.");
        }
        asn_dec_rval_t decode_rval = asn_decode(
            nullptr,
            ATS_UNALIGNED_BASIC_PER,
            &asn_DEF_MessageFrame,
            (void **)&messageFrame,
            byte_buffer.data(),
            byte_buffer.size());

        if (decode_rval.code != RC_OK)
        {
            erroross.str("");
            erroross << "failed ASN.1 binary decoding of element " << asn_DEF_MessageFrame.name << ": bad data. Successfully decoded " << decode_rval.consumed << " bytes.";
            throw TelematicBridgeException(erroross.str());
        }
    }

    int DynamicBufferAppend(const void *buffer, size_t size, void *app_key)
    {
        auto *xb = static_cast<buffer_structure_t *>(app_key);

        while (xb->buffer_size + size + 1 > xb->allocated_size)
        {
            // increase size of buffer.
            size_t new_size = 2 * (xb->allocated_size ? xb->allocated_size : 64);
            auto new_buf = static_cast<char *>(MALLOC(new_size));
            if (!new_buf)
                return -1;
            // move old to new.
            memcpy(new_buf, xb->buffer, xb->buffer_size);

            FREEMEM(xb->buffer);
            xb->buffer = new_buf;
            xb->allocated_size = new_size;
        }

        memcpy(xb->buffer + xb->buffer_size, buffer, size);
        xb->buffer_size += size;
        // null terminate the string.
        xb->buffer[xb->buffer_size] = '\0';
        return 0;
    }

    /**
     * @brief Convert the J2735 messageFrame into string in XML format
     * @param MessageFrame_t J2735 struct
     * @return string XML formatted J2735 message
     */
    string ConvertJ2735FrameToXML(const MessageFrame_t *messageFrame)
    {
        /**
         * Convert J2735 message into XML
         */
        buffer_structure_t xml_buffer = {nullptr, 0, 0};
        asn_enc_rval_t encode_rval = xer_encode(
            &asn_DEF_MessageFrame,
            messageFrame,
            XER_F_CANONICAL,
            DynamicBufferAppend,
            static_cast<void *>(&xml_buffer));
        if (encode_rval.encoded == -1)
        {
            throw TelematicBridgeException("Failed to  convert message with ID (=" + to_string(messageFrame->messageId) + ") to XML ");
        }
        auto output = string(xml_buffer.buffer);
        FREEMEM(xml_buffer.buffer);
        return output;
    }

    /**
     * @brief convert JSON value into string
     * @param JSON input Json::Value
     * @return string
     */
    string JsonToString(const Json::Value &json)
    {
        Json::FastWriter fasterWirter;
        string jsonStr = fasterWirter.write(json);
        boost::replace_all(jsonStr, "\\n", "");
        boost::replace_all(jsonStr, "\n", "");
        boost::replace_all(jsonStr, "\\t", "");
        boost::replace_all(jsonStr, "\\", "");
        return jsonStr;
    }
    /**
     * @brief convert string into JSON value
     * @param string input string
     * @return JSON::Value
     */
    Json::Value StringToJson(const string &str)
    {
        Json::Value root;
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(str, root);
        if (!parsingSuccessful)
        {
            throw TelematicBridgeException("Error parsing the string");
        }
        return root;
    }
    /**
     * @brief Convert XML string into JSON string
     */
    string xml2Json(const string &xml_str)
    {
        stringstream xmlss;
        xmlss << xml_str;
        pt::ptree root;
        pt::read_xml(xmlss, root);
        stringstream jsonss;
        pt::write_json(jsonss, root, false);
        return jsonss.str();
    }
    /**
     * @brief create JSON payload from given IVP message
     * @param IVPMessage V2xHub interval exchanged message
     * @return JSON value
     */
    Json::Value IvpMessageToJson(const IvpMessage *msg)
    {
        Json::Value json;
        if (msg->type)
        {
            json["type"] = msg->type;
        }

        if (msg->subtype)
        {
            json["subType"] = msg->subtype;
        }

        if (msg->dsrcMetadata)
        {
            json["channel"] = msg->dsrcMetadata->channel;
            json["psid"] = msg->dsrcMetadata->psid;
        }

        if (msg->encoding)
        {
            json["encoding"] = msg->encoding;
        }

        if (msg->source)
        {
            json["source"] = msg->source;
        }
        json["sourceId"] = msg->sourceId;
        json["flags"] = msg->flags;
        json["timestamp"] = msg->timestamp;
        if (msg->payload)
        {
            if (msg->payload->type == cJSON_Number)
            {
                json["payload"] = (msg->payload->valueint == 0 ? msg->payload->valuedouble : static_cast<double>(msg->payload->valueint));
            }
            else
            {
                // Render a cJSON entity to text for transfer/storage. Free the char* when finished.
                std::unique_ptr<char, decltype(std::free) *> payloadPtr{ cJSON_Print(msg->payload), std::free };    
                json["payload"] = StringToJson(payloadPtr.get());
            }
        }

        return json;
    }
} // TelematicBridge