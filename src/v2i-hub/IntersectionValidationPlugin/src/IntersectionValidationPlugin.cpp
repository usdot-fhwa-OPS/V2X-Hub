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
 
#include "IntersectionValidationPlugin.h"
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
 
namespace IntersectionValidation
{
    IntersectionValidationPlugin::IntersectionValidationPlugin(const std::string &name): PluginClientClockAware(name)
    {
 
        AddMessageFilter<SpatMessage>(this, &IntersectionValidationPlugin::HandleSpatMessage);
        AddMessageFilter<MapDataMessage>(this, &IntersectionValidationPlugin::HandleMapDataMessage);
        AddMessageFilter<TimMessage>(this, &IntersectionValidationPlugin::HandleTimMessage);
 
        SubscribeToMessages();
    }

    void IntersectionValidationPlugin::UpdateConfigSettings()
    {
        // TODO: Implement configuration setting update logic
    }

	void IntersectionValidationPlugin::OnConfigChanged(const char *key, const char *value)
	{
		// TODO: Implement configuration change handling
	}

	void IntersectionValidationPlugin::OnMessageReceived(IvpMessage *msg)
	{
		// TODO: Implement message receiving handling
	}

	void IntersectionValidationPlugin::OnStateChange(IvpPluginState state)
	{
		// TODO: Implement state change handling
	}
 
    void IntersectionValidationPlugin::HandleSpatMessage(SpatMessage &msg, routeable_message &routeableMsg)
    {
        // TODO: Perform SPAT required fields validation
    }
 
    void IntersectionValidationPlugin::HandleMapDataMessage(MapDataMessage &msg, routeable_message &routeableMsg)
    {
        // TODO: Perform MAP required fields validation
    }
 
    void IntersectionValidationPlugin::HandleTimMessage(TimMessage &msg, routeable_message &routeableMsg)
    {
        // TODO: Perform TIM required fields validation
    }
 
}
 
int main(int argc, char *argv[])
{
    return run_plugin<IntersectionValidation::IntersectionValidationPlugin>("IntersectionValidationPlugin", argc, argv);
}