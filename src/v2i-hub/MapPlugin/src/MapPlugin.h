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
#include <atomic>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <time.h>
#include <sys/time.h>
#include <string>
#include <cstdio>
#include <chrono>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include <tmx/IvpPlugin.h>
#include <tmx/messages/IvpBattelleDsrc.h>
#include <tmx/messages/IvpSignalControllerStatus.h>
#include <tmx/messages/IvpJ2735.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>

#define USE_STD_CHRONO
#include <FrequencyThrottle.h>
#include <PluginClientClockAware.h>

#include <MapSupport.h>

namespace MapPlugin {



class MapFile: public tmx::message {
public:
	using tmx::message::message;
    ~MapFile() override = default;

    MapFile(MapFile&&) noexcept = default;
    MapFile& operator=(MapFile&&) noexcept = default;

    MapFile(const MapFile&) = default;
    MapFile& operator=(const MapFile&) = default;

	std_attribute(this->msg, int, Action, -1, );
	std_attribute(this->msg, std::string, FilePath, "", );
	std_attribute(this->msg, std::string, InputType, "", );
	std_attribute(this->msg, std::string, Bytes, "", );

	static tmx::message_tree_type to_tree(const MapFile &m) {
		return tmx::message::to_tree(static_cast<const tmx::message&>(m));
	}

	static MapFile from_tree(const tmx::message_tree_type &tree) {
		MapFile m;
		m.set_contents(tree);
		return m;
	}
};

class MapPlugin: public tmx::utils::PluginClientClockAware {
public:
	explicit MapPlugin(const std::string &name);
	int Main() override;

protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value) override;
	void OnMessageReceived(IvpMessage *msg) override;
	void OnStateChange(IvpPluginState state) override;

	bool LoadMapFiles();
	void DebugPrintMapFiles();

	std::string enum_to_hex_string();
	std::string removeMessageFrame(const std::string &fileContent);
	std::string checkMapContent(const std::string &fn);

private:
	tmx::messages::J2735MessageFactory factory;
	tmx::utils::FrequencyThrottle<int> errThrottle;

	std::atomic<int> _mapAction {-1};
	std::atomic<bool> _isMapFileNew {false};
	std::atomic<bool> _cohdaR63 {false};
	/** Status key for if Map File loaded successfully */
	const char* _keyMapFileStatus = "Map File Loaded";

	std::mutex data_lock;
	std::map<int, MapFile> _mapFiles;
	int sendFrequency = 1000;

	std::array<char, 5> mapID_buffer;
};

} // namespace MapPlugin
