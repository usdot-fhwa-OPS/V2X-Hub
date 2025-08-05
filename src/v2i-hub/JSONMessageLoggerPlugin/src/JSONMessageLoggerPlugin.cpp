/**
 * Copyright (C) 2025 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
#include "JSONMessageLoggerPlugin.hpp"

namespace JSONMessageLoggerPlugin {
    JSONMessageLoggerPlugin::JSONMessageLoggerPlugin(const std::string &name) : tmx::utils::TmxMessageManager(name)
    {
        AddMessageFilter("*", "*", IvpMsgFlags_None);
        AddMessageFilter("J2735", "*", IvpMsgFlags_RouteDSRC);
        SubscribeToMessages();
    }

    void JSONMessageLoggerPlugin::OnStateChange(IvpPluginState state)
    {
        tmx::utils::TmxMessageManager::OnStateChange(state);
		if (state == IvpPluginState_registered) {
			UpdateConfigSettings();
		}
    }

    void JSONMessageLoggerPlugin::OnConfigChanged(const char *key, const char *value)
    {
        tmx::utils::TmxMessageManager::OnConfigChanged(key, value);
		UpdateConfigSettings();
    }
    void JSONMessageLoggerPlugin::OnMessageReceived(tmx::routeable_message &msg)
    {
        tmx::utils::TmxMessageManager::OnMessageReceived(msg);
        // Cast routeable message as J2735 Message
        if (tmx::utils::PluginClient::IsJ2735Message(msg)) {
            tmx::messages::TmxJ2735EncodedMessage<tmx::messages::MessageFrameMessage> j2735Msg;
            j2735Msg.set_data(msg.get_payload_bytes());
            // Get J2735 ASN.1 C Struct 
            auto j2735Data = j2735Msg.decode_j2735_message().get_j2735_data();
            // Call JER Print function on message frame
            jer_fprint(stdout, &asn_DEF_MessageFrame, j2735Data.get());
            /**
             * Convert J2735 message into XML
             */
            std::string *json_buffer;
            
            asn_enc_rval_t encode_rval = jer_encode(
                &asn_DEF_MessageFrame,
                j2735Data.get(),
                jer_encoder_flags_e::JER_F_MINIFIED,
                DynamicBufferAppend,
                &json_buffer // Pass the buffer to the callback
               );

            PLOG(tmx::utils::LogLevel::logINFO) << "Encoded J2735 message to JSON: " << json_buffer->c_str();
            
            // auto output = string(json_buffer.buffer);
            // asn_fprint(stdout, &asn_DEF_MessageFrame, frame.get_j2735_data().get());
            ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_MessageFrame, j2735Data.get());
            // auto decoded_msg = tmx::messages::TmxJ2735EncodedMessage<tmx::messages::MessageFrameMessage>::decode_j2735_message();
            // Copy into TmxJ2735EncodedMessage
            // tmx::messages::TmxJ2735EncodedMessageBase *j2735Msg = dynamic_cast<tmx::messages::TmxJ2735EncodedMessageBase *>(&msg);
            // if (j2735Msg) {
            //     j2735Msg->get
            // }

        }
        

        
    }

    void JSONMessageLoggerPlugin::UpdateConfigSettings()
    {
       
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
} // namespace JSONMessageLoggerPlugin
// The main entry point for this application.
int main(int argc, char *argv[])
{
	return tmx::utils::run_plugin<JSONMessageLoggerPlugin::JSONMessageLoggerPlugin>("JSON Message Logger Plugin", argc, argv);
}