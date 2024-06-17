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
#include "MUSTSensorDriverPlugin.h"

using namespace tmx::utils;
using namespace std;

namespace MUSTSensorDriverPlugin {
	
	MUSTSensorDriverPlugin::MUSTSensorDriverPlugin(const string &name): PluginClientClockAware(name)
	{

		// Subscribe to all messages specified by the filters above.
		SubscribeToMessages();
	}

	

	void MUSTSensorDriverPlugin::UpdateConfigSettings()
	{
		// Configuration settings are retrieved from the API using the GetConfigValue template class.
		// This method does NOT execute in the main thread, so variables must be protected
		// (e.g. using std::atomic, std::mutex, etc.).
	}


	void MUSTSensorDriverPlugin::OnConfigChanged(const char *key, const char *value)
	{
		PluginClientClockAware::OnConfigChanged(key, value);
		UpdateConfigSettings();
	}


} /* namespace MUSTSensorDriver */

int main(int argc, char *argv[])
{
	return run_plugin<MUSTSensorDriverPlugin::MUSTSensorDriverPlugin>("MUSTSensorDriverPlugin", argc, argv);
}
