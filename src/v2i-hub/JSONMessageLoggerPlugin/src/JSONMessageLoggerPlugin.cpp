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
            // asn_fprint(stdout, &asn_DEF_MessageFrame, frame.get_j2735_data().get());
            free(j2735Data.get());
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

} // namespace JSONMessageLoggerPlugin
// The main entry point for this application.
int main(int argc, char *argv[])
{
	return tmx::utils::run_plugin<JSONMessageLoggerPlugin::JSONMessageLoggerPlugin>("JSON Message Logger Plugin", argc, argv);
}