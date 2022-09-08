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

#include <tmx/tmx.h>
#include <tmx/IvpPlugin.h>
#include <tmx/messages/IvpBattelleDsrc.h>
#include <tmx/messages/IvpSignalControllerStatus.h>
#include <tmx/messages/IvpJ2735.h>
#include <tmx/messages/byte_stream.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>
#include "XmlMapParser.h"
#include "ConvertToJ2735r41.h"
#include "inputs/isd/ISDToJ2735r41.h"

#define USE_STD_CHRONO
#include <FrequencyThrottle.h>
#include <PluginClient.h>

#include "utils/common.h"
#include "utils/map.h"

#include <MapSupport.h>
using namespace std;
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
	virtual ~MapFile() {}

	std_attribute(this->msg, int, Action, -1, );
	std_attribute(this->msg, std::string, FilePath, "", );
	std_attribute(this->msg, std::string, InputType, "", );
	std_attribute(this->msg, std::string, Bytes, "", );
public:
	static tmx::message_tree_type to_tree(MapFile m) {
		return tmx::message::to_tree(static_cast<tmx::message>(m));
	}

	static MapFile from_tree(tmx::message_tree_type tree) {
		MapFile m;
		m.set_contents(tree);
		return m;
	}
};

//int _mapAction = -1;
//bool _isMapFilesNew = false;
//bool _isMapLoaded = false;

volatile int gMessageCount = 0;

class MapPlugin: public PluginClient {
public:
	MapPlugin(string name);
	virtual ~MapPlugin();

	virtual int Main();
protected:
	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnMessageReceived(IvpMessage *msg);
	void OnStateChange(IvpPluginState state);
private:
	std::atomic<int> _mapAction {-1};
	std::atomic<bool> _isMapFileNew {false};
	std::atomic<bool> _cohdaR63 {false};

	std::map<int, MapFile> _mapFiles;
	std::mutex data_lock;

	J2735MessageFactory factory;

	FrequencyThrottle<int> throttle;
	FrequencyThrottle<int> errThrottle;

	bool LoadMapFiles();
	void DebugPrintMapFiles();
};