
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

MapPlugin::MapPlugin(string name) :
		PluginClient(name) {
	AddMessageFilter(IVPMSG_TYPE_SIGCONT, "ACT", IvpMsgFlags_None);
	SubscribeToMessages();
	errThrottle.set_Frequency(std::chrono::minutes(30));
}

MapPlugin::~MapPlugin() {

}

void MapPlugin::UpdateConfigSettings() {
	int gFrequency;
	GetConfigValue("Frequency", gFrequency);
	throttle.set_Frequency(chrono::milliseconds(gFrequency));

	message_tree_type rawMapFiles;
	GetConfigValue("MAP_Files", rawMapFiles);

	if (!rawMapFiles.empty()) {
		try
		{
			lock_guard<mutex> lock(data_lock);
			_mapFiles.clear();

			tmx::message mapFiles;
			mapFiles.set_contents(rawMapFiles);

			PLOG(logDEBUG) << "Got MAP_Files: " << mapFiles;

			for (auto mapFile : mapFiles.template get_array<MapFile>("MapFiles"))
			{
				if (mapFile.get_Action() < 0)
					continue;

				_mapFiles[mapFile.get_Action()] = mapFile;
				_isMapFileNew = true;
			}

			// Check to see if the active map was lost
			if (!_mapFiles.count(_mapAction))
			{
				if (_mapAction > 0)
				{
					PLOG(logINFO) << "New configuration does not contain a map for active action " <<
							_mapAction << ". Using default action.";
				}
				_mapAction = -1;
			}

			if (_mapFiles.size() > 0 && _mapAction < 0)
				_mapAction = _mapFiles.begin()->first;

		}
		catch (exception &ex)
		{
			PLOG(logERROR) << "Unable to parse map file input: " << ex.what();
		}

		DebugPrintMapFiles();
	}

}

void MapPlugin::OnConfigChanged(const char *key, const char *value) {
	PluginClient::OnConfigChanged(key, value);

	if (_plugin->state == IvpPluginState_registered)
	{
		// Check for special case Cohda R63 messages
		if (strcmp("Cohda R63", key))
		{
			string strValue(value);

			if (boost::iequals(strValue, "1")
				|| boost::iequals(strValue, "true")
				|| boost::iequals(strValue, "t")
				|| boost::iequals(strValue, "on"))
			{
				_cohdaR63 = true;
			}
			else
			{
				_cohdaR63 = false;
			}
		}
		else
		{
			UpdateConfigSettings();
		}
	}
}

void MapPlugin::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered) {
		UpdateConfigSettings();
	}
}

void MapPlugin::OnMessageReceived(IvpMessage *msg) {
	PluginClient::OnMessageReceived(msg);

	if ((strcmp(msg->type, IVPMSG_TYPE_SIGCONT) == 0)
			&& (strcmp(msg->subtype, "ACT") == 0)
			&& (msg->payload->type == cJSON_String)) {
		int action = ivpSigCont_getIvpSignalControllerAction(msg);

		if (action != _mapAction)
		{
			// Ignore if there is no map for this action
			lock_guard<mutex> lock(data_lock);
			if (_mapFiles.count(action) <= 0)
			{
				if (errThrottle.Monitor(action))
				{
					PLOG(logERROR) << "Missing map for Action " << action;
				}
				return;
			}

			_isMapFileNew = _mapAction.exchange(action) != action;
		}
	}
}

int MapPlugin::Main() {
	PLOG(logINFO) << "Starting plugin.";

	bool mapFilesOk = false;

	std::unique_ptr<MapDataEncodedMessage> msg;
	int activeAction = -1;

	while (_plugin->state != IvpPluginState_error) {
		if (_isMapFileNew) {
			msg.reset();
			activeAction = -1;

			mapFilesOk = LoadMapFiles();
			_isMapFileNew = false;
		}

		int temp = _mapAction;
		if (temp < 0)
		{
			// No action set yet, so just wait
			sleep(1);
			continue;
		}

		// Action has changed, so retrieve the correct map
		if (temp != activeAction)
		{
			lock_guard<mutex> lock(data_lock);
			string byteStr = _mapFiles[temp].get_Bytes();
			if (!byteStr.empty())
			{
				msg.reset(dynamic_cast<MapDataEncodedMessage *>(factory.NewMessage(api::MSGSUBTYPE_MAPDATA_STRING)));
				if (!msg)
				{	if (errThrottle.Monitor(temp))
					{
						PLOG(logERROR) << "Unable to create map from bytes " << byteStr << ": " << factory.get_event();
					}
					sleep(1);
					continue;
				}

				string enc = msg->get_encoding();
				msg->refresh_timestamp();
				msg->set_payload(byteStr);
				msg->set_encoding(enc);
				msg->set_flags(IvpMsgFlags_RouteDSRC);
				msg->addDsrcMetadata(172, 0x8002);

				activeAction = temp;
				PLOG(logINFO) << "Map for action " << activeAction << " will be sent";
			}
		}

		if (mapFilesOk && throttle.Monitor(0))
		{
			// Time to send a new message
			routeable_message *rMsg = dynamic_cast<routeable_message *>(msg.get());
			if (_cohdaR63)
			{
				auto bytes = rMsg->get_payload_bytes();
				rMsg->set_payload_bytes(bytes); // TODO: Translate to R63 bytes
			}

			if (rMsg) {
				rMsg->refresh_timestamp();
				BroadcastMessage(*rMsg);
			}
		}

		// Wake up a few times before next cycle, in case there is something to do
		usleep(1000 * throttle.get_Frequency().count() / 5);
	}

	return (EXIT_SUCCESS);
}

bool MapPlugin::LoadMapFiles()
{
	if (_mapFiles.empty())
		return false;

	lock_guard<mutex> lock(data_lock);
	for (auto &mapPair : _mapFiles)
	{
		MapFile &mapFile = mapPair.second;
		if (mapFile.get_Bytes() == "")
		{
			// Fill in the bytes for each map file
			string inType = mapFile.get_InputType();
			if (inType.empty())
			{
				try
				{
					string fn = mapFile.get_FilePath();

					if (fn.substr(fn.size() - 5) == ".json")
						inType = "ISD";
					else if (fn.substr(fn.size() - 4) == ".txt")
						inType = "TXT";
					else if (fn.substr(fn.size()- 5) == ".uper")
						inType ="UPER";
					else
						inType = "XML";

					if (inType == "ISD")
					{
						ISDToJ2735r41 converter(fn);
						mapFile.set_Bytes(converter.to_encoded_message().get_payload_str());

						PLOG(logINFO) << fn << " ISD file encoded as " << mapFile.get_Bytes();
					}
					else if (inType == "TXT")
					{
						byte_stream bytes;
						ifstream in(fn);
						in >> bytes;

						PLOG(logINFO) << fn << " MAP encoded bytes are " << bytes;

						MapDataMessage *mapMsg = MapDataEncodedMessage::decode_j2735_message<codec::uper<MapDataMessage> >(bytes);
						if (mapMsg) {
							PLOG(logDEBUG) << "Map is " << *mapMsg;

							MapDataEncodedMessage mapEnc;
							mapEnc.encode_j2735_message(*mapMsg);
							mapFile.set_Bytes(mapEnc.get_payload_str());

							PLOG(logINFO) << fn << " J2735 message bytes encoded as " << mapFile.get_Bytes();
						}
					}
					else if (inType == "UPER")
					{
						PLOG(logDEBUG) << "Reading MAP file as UPER encoded hex bytes including MessageFrame." << std::endl;
						std::ifstream in; 
						try {
							in.open(fn, std::ios::in | std::ios::binary );
							if (in.is_open()) {
								in.seekg(0, std::ios::end);
								int fileSize = in.tellg();
								in.seekg(0, std::ios::beg);
								PLOG(logDEBUG) << "File size is " << fileSize <<std::endl;
								std::string bytes_string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
								PLOG(logDEBUG) << "File contents : " << bytes_string << std::endl;
								mapFile.set_Bytes(bytes_string);								
							}
							else {
								PLOG(logERROR) << "Failed to open file " << fn << "." << std::endl;
							}
						}
						catch( const ios_base::failure &e) {
							PLOG(logERROR) << "Exception Encountered : \n" << e.what();
						}
					}
					else if (inType == "XML")
					{
						tmx::message_container_type container;
						container.load<XML>(fn);

						if (container.get_storage().get_tree().begin()->first == "MapData")
						{
							MapDataMessage mapMsg;
							mapMsg.set_contents(container.get_storage().get_tree());

							PLOG(logDEBUG) << "Encoding " << mapMsg;
							MapDataEncodedMessage mapEnc;
							mapEnc.encode_j2735_message(mapMsg);
							mapFile.set_Bytes(mapEnc.get_payload_str());

							PLOG(logINFO) << fn << " XML file encoded as " << mapFile.get_Bytes();
						}
						else
						{
							ConvertToJ2735r41 mapConverter;
							XmlMapParser mapParser;
							map theMap;

							if (mapParser.ReadGidFile(fn, &theMap))
							{
								mapConverter.convertMap(&theMap);

								PLOG(logDEBUG) << "Encoded Bytes:" << mapConverter.encodedByteCount;

								if (mapConverter.encodedByteCount > 0)
								{
									byte_stream bytes(mapConverter.encodedByteCount);
									memcpy(bytes.data(), mapConverter.encoded, mapConverter.encodedByteCount);

									auto *mapEnc = factory.NewMessage(bytes);
									if (!mapEnc)
										return false;

									mapFile.set_Bytes(mapEnc->get_payload_str());

									PLOG(logINFO) << fn << " input file encoded as " << mapEnc->get_payload_str();
								}
								else
								{
									return false;
								}
							}
						}
					}
				}
				catch (exception &ex)
				{
					PLOG(logERROR) << "Unable to convert " << mapFile.get_FilePath() << ": " << ex.what();
					return false;
				}
			}
		}
	}

	return true;
}

void MapPlugin::DebugPrintMapFiles() {
	PLOG(logDEBUG) << _mapFiles.size()
			<< " map files specified by configuration settings:";

	for (auto iter = _mapFiles.begin(); iter != _mapFiles.end(); iter++) {
		int key = iter->first;
		PLOG(logDEBUG) << "-- Action " << key << " file is " << iter->second.get_FilePath();
	}
}

} /* End namespace MapPlugin */

int main(int argc, char *argv[]) {
	return run_plugin<MapPlugin::MapPlugin>("MapPlugin", argc, argv);
}
