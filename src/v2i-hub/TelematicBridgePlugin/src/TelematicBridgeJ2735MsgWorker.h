#pragma once
#include "PluginLog.h"
#include <tmx/messages/TmxJ2735.hpp>
#include "TelematicBridgeException.h"

using namespace tmx::utils;
using namespace std;
using namespace tmx::messages;

namespace TelematicBridge
{
    using buffer_structure_t = struct buffer_structure
    {
        char *buffer;          // buffer array
        size_t buffer_size;    // this is really where we will write next.
        size_t allocated_size; // this is the total size of the buffer.
    };

    class TelematicBridgeJ2735MsgWorker
    {
    public:
        /**
         * @brief Convert hex string into a vector of bytes
         * @param string input hex string payload
         * @param vector<char*> byte buffer to be updated.
         * @return bool indicator whether conversion is successful.
         */
        static bool HexToBytes(const string &hexPaylod, vector<char> &byteBuffer);

        /**
         * @brief Decode J2735 message and populate the J2735 data frame
         * @param string input hex string payload
         * @param MessageFrame_t J2735 struct to be updated
         * @return bool indicator whether decoding successful
         */
        static void DecodeJ2735Msg(const string &hexPaylod, MessageFrame_t *messageframe);
        /**
         * @brief Convert the J2735 messageFrame into string in XML format
         * @param MessageFrame_t J2735 struct
         * @return string XML formatted J2735 message
         */
        static string ConvertJ2735FrameToXML(const MessageFrame_t *messageframe);
        static int dynamic_buffer_append(const void *buffer, size_t size, void *app_key);
        TelematicBridgeJ2735MsgWorker() = delete;
        ~TelematicBridgeJ2735MsgWorker() = delete;
    };

} // TelematicBridge
