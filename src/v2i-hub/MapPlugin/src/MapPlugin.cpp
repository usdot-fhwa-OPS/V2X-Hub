#include "MapPlugin.h"

using namespace tmx::utils;

namespace MapPlugin {

	MapPlugin::MapPlugin(const std::string &name) : PluginClientClockAware(name) {
		AddMessageFilter(IVPMSG_TYPE_SIGCONT, "ACT", IvpMsgFlags_None);
		SubscribeToMessages();
		errThrottle.set_Frequency(std::chrono::minutes(30));
		
	}

	void MapPlugin::UpdateConfigSettings() {
		GetConfigValue("Frequency", sendFrequency);

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
				std::string strValue(value);

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
		PluginClientClockAware::OnStateChange(state);

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

		std::unique_ptr<tmx::messages::MapDataEncodedMessage> msg;
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
				std::string byteStr = _mapFiles[temp].get_Bytes();
				if (!byteStr.empty())
				{
					msg.reset(dynamic_cast<tmx::messages::MapDataEncodedMessage *>(factory.NewMessage(tmx::messages::api::MSGSUBTYPE_MAPDATA_STRING)));
					if (!msg)
					{	if (errThrottle.Monitor(temp))
						{
							PLOG(logERROR) << "Unable to create map from bytes " << byteStr << ": " << factory.get_event();
						}
						sleep(1);
						continue;
					}

					std::string enc = msg->get_encoding();
					msg->refresh_timestamp();
					msg->set_payload(byteStr);
					msg->set_encoding(enc);
					msg->set_flags(IvpMsgFlags_RouteDSRC);
					msg->addDsrcMetadata(tmx::messages::api::mapData_PSID);

					activeAction = temp;
					PLOG(logINFO) << "Map for action " << activeAction << " will be sent";
				}
			}

			if (mapFilesOk)
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

			auto sleepUntil = getClock()->nowInMilliseconds() + sendFrequency;
			getClock()->sleep_until(sleepUntil);
		}

		return (EXIT_SUCCESS);
	}

	std::string MapPlugin::enum_to_hex_string()
	{
		std::snprintf(mapID_buffer.data(), mapID_buffer.size(), "%04X", tmx::messages::api::mapData);
		std::string map_messageID(mapID_buffer.data());

		return map_messageID;
	}

	std::string MapPlugin::removeMessageFrame(const std::string &fileContent)
	{
		std::string map_messageID = enum_to_hex_string();

		// Check for and remove MessageFrame
		if (fileContent.size() >= 4 && fileContent.substr(0, 4) == map_messageID)
		{
			// Check if message is hex size > 255, remove appropriate header
			std::string tempFrame = fileContent;
			std::string newFrame = fileContent;
			tempFrame.erase(0, 6);
			PLOG(logDEBUG4) << "Checking size of: " << tempFrame;
			auto headerSize = (tempFrame.size() > 510) ? 8 : 6;
			newFrame.erase(0, headerSize);
			
			PLOG(logDEBUG4) << "Payload without MessageFrame: " << newFrame;
			return newFrame;
		}
		else
		{
			return fileContent;
		}
	}

	std::string MapPlugin::checkMapContent(const std::string &fn)
	{
		PLOG(logDEBUG4) << "In MapPlugin :: checkMapContent";
		try
		{
			std::ifstream in(fn.c_str(), std::ios::binary);
			if (!in)
			{
				PLOG(logERROR) << "Failed to open file: " << fn.c_str();
				throw std::ios_base::failure("Failed to open file: " + fn);
			}
			else
			{
				std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
				in.close();
				// Remove any newline characters
				content.erase(remove(content.begin(), content.end(), '\n'), content.end());
				PLOG(logDEBUG4) << "Map without newline " << content;
				std::string payload = removeMessageFrame(content);

				return payload;
			}
		}
		catch (const std::ios_base::failure& e)
		{
			PLOG(logERROR) << "Exception encountered while reading file: " << e.what();
			throw;
		}
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
				std::string inType = mapFile.get_InputType();
				if (inType.empty())
				{
					try
					{
						std::string fn = mapFile.get_FilePath();

						if (fn.substr(fn.size() - 5) == ".json")
							inType = "ISD";
						else if (fn.substr(fn.size() - 4) == ".txt")
							inType = "TXT";
						else if (fn.substr(fn.size() - 4) == ".xml")
							inType = "XML";
						else 
							PLOG(logWARNING) << "Incorrect MapFile extension entered!";

						if (inType == "ISD")
						{
							ISDToJ2735r41 converter(fn);
							mapFile.set_Bytes(converter.to_encoded_message().get_payload_str());

							PLOG(logINFO) << fn << " ISD file encoded as " << mapFile.get_Bytes();
						}
						else if (inType == "TXT")
						{
							std::string payload = checkMapContent(fn);
							byte_stream bytes;
							std::istringstream streamableContent(payload);
							streamableContent >> bytes;
							PLOG(logINFO) << "MAP encoded bytes are " << bytes;
							tmx::messages::MapDataMessage *mapMsg = tmx::messages::MapDataEncodedMessage::decode_j2735_message<tmx::messages::codec::uper<tmx::messages::MapDataMessage>>(bytes);

							if (mapMsg)
							{
								PLOG(logDEBUG) << "Map is " << *mapMsg;

								tmx::messages::MapDataEncodedMessage mapEnc;
								mapEnc.encode_j2735_message(*mapMsg);
								mapFile.set_Bytes(mapEnc.get_payload_str());

								PLOG(logINFO) << "J2735 message bytes encoded as " << mapFile.get_Bytes();
							}
						}
						else if (inType == "XML")
						{
							tmx::message_container_type container;
							container.load<XML>(fn);

							if (container.get_storage().get_tree().begin()->first == "MapData")
							{
								tmx::messages::MapDataMessage mapMsg;
								mapMsg.set_contents(container.get_storage().get_tree());

								PLOG(logDEBUG) << "Encoding " << mapMsg;
								tmx::messages::MapDataEncodedMessage mapEnc;
								mapEnc.encode_j2735_message(mapMsg);
								mapFile.set_Bytes(mapEnc.get_payload_str());

								PLOG(logINFO) << fn << " XML file encoded as: " << mapFile.get_Bytes();
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

										PLOG(logINFO) << fn << " input file encoded as: " << mapEnc->get_payload_str();
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
