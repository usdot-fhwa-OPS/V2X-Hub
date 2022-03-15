
#ifndef SAMPLEBSMPLUGIN_H
#define SAMPLEBSMPLUGIN_H
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>
#include <thread>
#include <time.h>
#include <tmx/IvpPlugin.h>
#include <tmx/messages/IvpJ2735.h>
#include <tmx/tmx.h>
#include <unistd.h>

#include "PluginClient.h"
#include "PluginUtil.h"

#include <ApplicationDataMessage.h>
#include <ApplicationMessage.h>

#include <tmx/messages/IvpJ2735.h>
#include <tmx/messages/auto_message.hpp>

#include <tmx/j2735_messages/BasicSafetyMessage.hpp>

using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;

namespace SampleBSMPluginNS
{
    class SampleBSMPlugin : public PluginClient
    {
    public:
        SampleBSMPlugin(std::string name);
        ~SampleBSMPlugin();
        void HandleBasicSafetyMessage(BsmMessage &msg, routeable_message &routeableMsg);
        int Main();
        void EncodeBSM(BsmEncodedMessage& bsmEncodedMessage) const;
        void EncodeBroadcastBSM();
    };

}
#endif