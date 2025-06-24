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
#pragma once
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <PluginClientClockAware.h>


#include <tmx/j2735_messages/BasicSafetyMessage.hpp>

using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;

namespace SampleBSMPluginNS
{
    class SampleBSMPlugin : public PluginClientClockAware
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