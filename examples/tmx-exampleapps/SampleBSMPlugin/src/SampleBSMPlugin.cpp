/**
 * Copyright (C) 2024 LEIDOS.
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

#include "SampleBSMPlugin.h"
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;

namespace SampleBSMPluginNS
{
    SampleBSMPlugin::SampleBSMPlugin(std::string name) : PluginClientClockAware(name)
    {
        AddMessageFilter<BsmMessage>(this, &SampleBSMPlugin::HandleBasicSafetyMessage);
        SubscribeToMessages();

        //Create and broadcast new BSM
        std::thread t1(&SampleBSMPlugin::EncodeBroadcastBSM, this);
        t1.join();
    }

    void SampleBSMPlugin::HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg)
    {
        //Getting BSM from MessageReceiver plugin
        auto bsm = msg.get_j2735_data();  
             
        BsmEncodedMessage bsmEncodeMessage;
      
        BsmMessage* bsmMessage = new BsmMessage(bsm);        
        MessageFrameMessage frame_msg(bsmMessage->get_j2735_data());
        bsmEncodeMessage.set_data(TmxJ2735EncodedMessage<BasicSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(frame_msg));   
        bsmEncodeMessage.set_flags(IvpMsgFlags_RouteDSRC);
        bsmEncodeMessage.addDsrcMetadata(0x20);
        bsmEncodeMessage.refresh_timestamp();

        routeable_message *rMsg = dynamic_cast<routeable_message *>(&bsmEncodeMessage);
        BroadcastMessage(*rMsg);
        free(frame_msg.get_j2735_data().get()); 
        delete bsmMessage;
        
    }

    void SampleBSMPlugin::EncodeBroadcastBSM()
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(2000)); 

            if(IsPluginState(IvpPluginState_error) )
            {
                break;
            }

            if (IsPluginState(IvpPluginState_disconnected) || !IsPluginState(IvpPluginState_registered))
            {
                continue;
            }

            //Create new BSM                 
            BsmEncodedMessage bsmEncoded;
            EncodeBSM( bsmEncoded );

            //Broadcast BSM
            bsmEncoded.set_flags(IvpMsgFlags_RouteDSRC);
            bsmEncoded.addDsrcMetadata(0x20);
            bsmEncoded.refresh_timestamp();
            routeable_message *rMsg = dynamic_cast<routeable_message *>(&bsmEncoded);
            BroadcastMessage(*rMsg);
            PLOG(logINFO) << "Broadcasted CREATED BSM : " << bsmEncoded.get_payload_str();
        }       
    }

    void SampleBSMPlugin::EncodeBSM(BsmEncodedMessage& bsmEncoded) const
    {   
        BasicSafetyMessage_t* bsm = (BasicSafetyMessage_t*) calloc(1, sizeof(BasicSafetyMessage_t) );  
        char* my_str = (char *) "sender_id";
        uint8_t* my_bytes = reinterpret_cast<uint8_t *>(my_str);
        bsm->coreData.msgCnt = 1;
        uint8_t  my_bytes_id[4] = {(uint8_t)1, (uint8_t)12, (uint8_t)12, (uint8_t)10};
        bsm->coreData.id.buf = my_bytes_id;
        bsm->coreData.id.size = sizeof(my_bytes_id);
        bsm->coreData.secMark = 1023;
        bsm->coreData.lat = 38954961;
        bsm->coreData.Long = -77149303;
        bsm->coreData.elev = 72;
        bsm->coreData.speed = 100;
        bsm->coreData.heading = 12;
        bsm->coreData.angle = 10;
        bsm->coreData.transmission = 0;          

        //position accuracy
        bsm->coreData.accuracy.orientation= 100;
        bsm->coreData.accuracy.semiMajor = 200;
        bsm->coreData.accuracy.semiMinor = 200;

        //Acceleration set
        bsm->coreData.accelSet.lat = 100;
        bsm->coreData.accelSet.Long = 300;
        bsm->coreData.accelSet.vert = 100;
        bsm->coreData.accelSet.yaw = 0;

        //populate brakes
        bsm->coreData.brakes.abs = 1; 
        bsm->coreData.brakes.scs = 1; 
        bsm->coreData.brakes.traction = 1; 
        bsm->coreData.brakes.brakeBoost = 1; 
        bsm->coreData.brakes.auxBrakes = 1; 
        uint8_t  my_bytes_brakes[1] = {8};
        bsm->coreData.brakes.wheelBrakes.buf = my_bytes_brakes; 
        bsm->coreData.brakes.wheelBrakes.size = sizeof(my_bytes_brakes); 
        bsm->coreData.brakes.wheelBrakes.bits_unused = 3; 

        //vehicle size
        bsm->coreData.size.length = 500;
        bsm->coreData.size.width = 300;

        PLOG(logINFO) << "Create and Send BSM." ;         
        BsmMessage*  bsmMessage = new BsmMessage(bsm);
        MessageFrameMessage messageFrame( bsmMessage->get_j2735_data() );
        bsmEncoded.set_data( TmxJ2735EncodedMessage<BasicSafetyMessage>::encode_j2735_message<codec::uper<MessageFrameMessage>>(messageFrame) );
        free(bsm);
        free(messageFrame.get_j2735_data().get());     
    }

    SampleBSMPlugin::~SampleBSMPlugin() {};

    int SampleBSMPlugin::Main()
    {
        PLOG(logINFO) << "Starting plugin";
        while (_plugin->state != IvpPluginState_error)
        {
            if (IsPluginState(IvpPluginState_registered))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));                
            }
        }
        return (EXIT_SUCCESS);
    }

}

int main(int argc, char *argv[])
{
    return run_plugin<SampleBSMPluginNS::SampleBSMPlugin>("SampleBSMPlugin", argc, argv);
}
