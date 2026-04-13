/**
 * Copyright (C) 2026 LEIDOS.
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
 
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <tmx/j2735_messages/SpatMessage.hpp>
#include <tmx/j2735_messages/TravelerInformationMessage.hpp>

 
namespace IntersectionValidation
{
 
    class IntersectionValidationPlugin : public tmx::utils::PluginClientClockAware
    {
    public:
        explicit IntersectionValidationPlugin(const std::string &name);
        ~IntersectionValidationPlugin() override = default;

        void UpdateConfigSettings();
 
        // Message handlers
        void HandleSpatMessage(tmx::messages::SpatMessage &msg, tmx::routeable_message &routeableMsg);
        void HandleMapDataMessage(tmx::messages::MapDataMessage &msg, tmx::routeable_message &routeableMsg);
        void HandleTimMessage(tmx::messages::TimMessage &msg, tmx::routeable_message &routeableMsg);

    protected:
        // Virtual method overrides.
        void OnConfigChanged(const char *key, const char *value) override;
        void OnMessageReceived(IvpMessage *msg) override;
        void OnStateChange(IvpPluginState state) override;
    };
}