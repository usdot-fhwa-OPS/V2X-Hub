//============================================================================
// Name        : MapPlugin.cpp
// Author      : FHWA Saxton Transportation Operations Laboratory
// Version     : 7.6.0
// Copyright   : Copyright (c) 2024 FHWA Saxton Transportation Operations Laboratory. All rights reserved.
// Description : MAP Plugin
//============================================================================

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

#include <tmx/tmx.h>
#include <tmx/IvpPlugin.h>
#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <MapData.h>
#include <tmx/messages/IvpBattelleDsrc.h>
#include <tmx/messages/IvpSignalControllerStatus.h>
#include <tmx/messages/IvpJ2735.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include <tmx/TmxApiMessages.h>
#include "XmlMapParser.h"
#include "ConvertToJ2735r41.h"
#include "inputs/isd/ISDToJ2735r41.h"

#define USE_STD_CHRONO
#include <FrequencyThrottle.h>
#include <PluginClientClockAware.h>

#include "utils/common.h"
#include "utils/map.h"

#include <MapSupport.h>

using namespace tmx;
using namespace tmx::messages;
using namespace tmx::utils;

namespace MapPlugin {

#if SAEJ2735_SPEC < 63
UPERframe _uperFrameMessage;
#endif

class MapFile: public tmx::message {
public:
	MapFile(): tmx::message() {}
	virtual ~MapFile() = default;

	std_attribute(this->msg, int, Action, -1, );
	std_attribute(this->msg, std::string, FilePath, "", );
	std_attribute(this->msg, std::string, InputType, "", );
	std_attribute(this->msg, std::string, Bytes, "", );

	static tmx::message_tree_type to_tree(MapFile& m) {
		return tmx::message::to_tree(static_cast<tmx::message>(m));
	}

	static MapFile from_tree(const tmx::message_tree_type &tree) {
		MapFile m;
		m.set_contents(tree);
		return m;
	}
};

class MapPlugin: public PluginClientClockAware {
public:
	explicit MapPlugin(const std::string &name);
	virtual ~MapPlugin() = default;
	int Main() override;

protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value) override;
	void OnMessageReceived(IvpMessage *msg) override;
	void OnStateChange(IvpPluginState state) override;

private:
	std::atomic<int> _mapAction {-1};
	std::atomic<bool> _isMapFileNew {false};
	std::atomic<bool> _cohdaR63 {false};

	std::map<int, MapFile> _mapFiles;
	std::mutex data_lock;

	J2735MessageFactory factory;

	int sendFrequency = 1000;
	FrequencyThrottle<int> errThrottle;

	std::array<char, 5> mapID_buffer;

	bool LoadMapFiles();
	void DebugPrintMapFiles();
	std::string enum_to_hex_string();
	std::string removeMessageFrame(const std::string &fileContent);
	std::string checkMapContent(std::ifstream &in, const std::string &fileName);

};

} // namespace MapPlugin
