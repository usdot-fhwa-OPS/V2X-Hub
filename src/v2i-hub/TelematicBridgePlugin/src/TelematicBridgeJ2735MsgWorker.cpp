#include "TelematicBridgeJ2735MsgWorker.h"

namespace TelematicBridge
{
    bool TelematicBridgeJ2735MsgWorker::HexToBytes(const string &hexPaylod, vector<char> &byteBuffer)
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

    void TelematicBridgeJ2735MsgWorker::DecodeJ2735Msg(const string &hexPaylod, MessageFrame_t* messageFrame)
    {
        /**
         * Decode J2735 message
         */
        ostringstream erroross;
        vector<char> byte_buffer;
        static constexpr size_t max_errbuf_size = 128;
        size_t errlen = max_errbuf_size;
        if (!HexToBytes(hexPaylod, byte_buffer))
        {
            throw TelematicBridgeException("Failed attempt to decode MessageFrame hex string: cannot convert to bytes.");
        }
        asn_dec_rval_t decode_rval = asn_decode(
            0,
            ATS_UNALIGNED_BASIC_PER,
            &asn_DEF_MessageFrame,
            (void **)&messageFrame,
            byte_buffer.data(),
            byte_buffer.size());

        if (decode_rval.code != RC_OK)
        {
            erroross.str("");
            erroross << "failed ASN.1 binary decoding of element " << asn_DEF_MessageFrame.name << ": ";
            if (decode_rval.code == RC_FAIL)
            {
                erroross << "bad data.";
            }
            else
            {
                erroross << "more data expected.";
            }
            erroross << " Successfully decoded " << decode_rval.consumed << " bytes.";
            throw TelematicBridgeException(erroross.str());
        }
    }

    string TelematicBridgeJ2735MsgWorker::ConvertJ2735FrameToXML(const MessageFrame_t *messageFrame)
    {
        /**
         * Convert J2735 message into XML
         */
        buffer_structure_t xml_buffer = {0, 0, 0};
        asn_enc_rval_t encode_rval = xer_encode(
            &asn_DEF_MessageFrame,
            messageFrame,
            XER_F_CANONICAL,
            dynamic_buffer_append,
            static_cast<void *>(&xml_buffer));
        if (encode_rval.encoded == -1)
        {
            throw TelematicBridgeException("Failed to  convert message with ID (=" + to_string(messageFrame->messageId) + ") to XML ");
        }
        return string(xml_buffer.buffer);
    }

    int TelematicBridgeJ2735MsgWorker::dynamic_buffer_append(const void *buffer, size_t size, void *app_key)
    {
        buffer_structure_t *xb = static_cast<buffer_structure_t *>(app_key);

        while (xb->buffer_size + size + 1 > xb->allocated_size)
        {
            // increase size of buffer.
            size_t new_size = 2 * (xb->allocated_size ? xb->allocated_size : 64);
            char *new_buf = static_cast<char *>(MALLOC(new_size));
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
} // TelematicBridge