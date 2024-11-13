//============================================================================
// Name        : CommandPlugin.h
// Author      : Battelle Memorial Institute - Ben Paselsky (paselsky@battelle.org)
// Version     :
// Copyright   : Copyright (c) 2017 Battelle Memorial Institute. All rights reserved.
// Description : Plugin that listens for websocket connections from the TMX admin portal
//				 and processes commands
//============================================================================

#ifndef TMX_PLUGINS_COMMANDPLUGIN_H_
#define TMX_PLUGINS_COMMANDPLUGIN_H_

#include <atomic>
#include <thread>
#include <queue>
#include <chrono>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <libwebsockets.h>

#include "PluginClient.h"
#include <EventLogMessage.h>
#include <tmx/messages/message_document.hpp>

#include <Base64.h>

#include <TmxControl.h>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>


using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace std;
using namespace tmxctl;
using namespace boost::property_tree;
using namespace boost::filesystem;
using namespace tmx::utils::telemetry;

namespace CommandPlugin
{

#define READ_BUFFER_SIZE 5000
#define MAX_SEND_BYTES 5000
#define DEFAULT_PLUGINDIRECTORY "/var/www/plugins"

/**
 * This plugin listens for websocket connections from the TMX admin portal
 * and processes commands
 */
class CommandPlugin: public PluginClient
{
public:
	CommandPlugin(std::string);
	virtual ~CommandPlugin();
	int Main();
protected:
	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	void OnStateChange(IvpPluginState state);
	void HandleEventLogMessage(EventLogMessage &msg, routeable_message &routeableMsg);

	void UpdateConfigSettings();
	static uint64_t GetMsTimeSinceEpoch();
	static int FileUploadCB(void *data, const char *name, const char *filename,
		       char *buf, int len, enum lws_spa_fileupload_states state);
	static void GetTelemetry(const string & dataType);
	static void GetEventTelemetry();
	static void BuildFullTelemetry(string *outputBuffer, const string & dataType);
	static void BuildUpdateTelemetry(string *outputBuffer, const string & dataType);
	static void BuildRemoveTelemetry(string *outputBuffer, const string & dataType);
	static void BuildCommandResponse(string *outputBuffer, const string & id, const string & command, const string & status, const string & reason, std::map<string, string> &data, std::map<string, string> &arrayData);
	static void SendData(string *outputBuffer, struct lws *wsi);

	static int WSCallbackHTTP(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
	static int WSCallbackBASE64(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

private:

	struct PerSessionDataBASE64
	{
		int bufferLength;
		char buffer[READ_BUFFER_SIZE];
		uint64_t lastUpdateSendTimeMS;
		uint64_t session;
		bool sendFullTelemetry;
		string *outputbuffer;
		uint64_t lastEventId;
		bool authorized;
		int authorizationLevel;
		uint64_t lastHeartbeatSendTimeMS;
	};

	struct PerSessionDataHTTP
	{
		struct lws_spa *spa;
			char result[LWS_PRE + 512];
			int result_len;

			char filename[64];
			long file_length;
		#if !defined(LWS_WITH_ESP32)
			lws_filefd_type fd;
		#endif
			int last_percent_sent;
	};

	struct EventLogMessage
	{
		string level;
		string source;
		string description;
		string timestamp;
	};

	struct UploadData
	{
		string requestId;
		uint64_t requestTimeMS;
		string *outputbuffer;
		string destinationFileName;
		string destinationPath;
		uint64_t fileSize;
		bool uploading;
		string message;
	};

	enum AuthorizationLevels
	{
		ReadOnly = 1,
		ApplicationAdministrator = 2,
		SystemAdministrator = 3
	};

	enum HTTPEnumParamNames {
		EPN_TEXT,
		EPN_SEND,
		EPN_FILE,
		EPN_UPLOAD,
	};

	static const char * _httpParamNames[4];

	atomic<bool> _newConfigValues{false};
	std::atomic<uint64_t> _sleepMS{10}; // sleep in milliseconds
	atomic<bool> _sslEnabled{false};
	string _sslPath = "";
	static std::atomic<uint64_t> _eventRowLimit; // limit on number of rows returned for initial event log query
	static string _downloadPath; // path to download directory
	static mutex _configLock;

	boost::lockfree::spsc_queue<EventLogMessage, boost::lockfree::capacity<1024> > _eventLogMessageQueue;

	static string _databaseAddress;
	static string _databasePort;
	static uint64_t _updateIntervalMS;
	static uint64_t _heartbeatIntervalMS;
	static uint64_t _lastPluginsUpdateTimeMS;
	static uint64_t _nextSession;
	static TmxControl _tmxControl;
	static uint64_t _connectionCount;
	static bool _haveList;
	static bool _haveConfig;
	static bool _haveStatus;
	static bool _haveState;
	static bool _haveMessages;
	static bool _haveSystemConfig;
	static bool _haveEvents;
	static tmx::message_container_type _listJSON;
	static string _configJSON;
	static string _statusJSON;
	static string _stateJSON;
	static string _messagesJSON;
	static string _systemConfigJSON;
	static string _eventsJSON;
	static string _lastEventTime;
	static std::map<string, string> _listPluginsJSON;
	static std::map<string, string> _configPluginsJSON;
	static std::map<string, string> _statusPluginsJSON;
	static std::map<string, string> _statePluginsJSON;
	static std::map<string, string> _messagesPluginsJSON;
	static std::map<string, string> _systemConfigPluginsJSON;
	static std::map<string, tmx::message_container_type> _listPluginsUpdatesJSON;
	static std::map<string, string> _configPluginsUpdatesJSON;
	static std::map<string, string> _statusPluginsUpdatesJSON;
	static std::map<string, string> _statePluginsUpdatesJSON;
	static std::map<string, string> _messagesPluginsUpdatesJSON;
	static std::map<string, string> _systemConfigPluginsUpdatesJSON;
	static std::map<string, string> _listPluginsRemoveJSON;
	static std::map<string, string> _configPluginsRemoveJSON;
	static std::map<string, string> _statusPluginsRemoveJSON;
	static std::map<string, string> _statePluginsRemoveJSON;
	static std::map<string, string> _messagesPluginsRemoveJSON;
	static std::map<string, string> _systemConfigPluginsRemoveJSON;
	static string _eventsUpdatesJSON;
	static string _eventsNextFullJSON;
	static uint64_t _eventsFullCount;
	static uint64_t _eventsNextFullCount;

	static std::map<string, UploadData> _uploadRequests;

};

} /* namespace CommandPlugin */

#endif /* TMX_PLUGINS_COMMANDPLUGIN_H_ */
