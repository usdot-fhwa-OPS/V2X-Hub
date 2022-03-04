//============================================================================
// Name        : CommandControl.cpp
// Author      : Battelle Memorial Institute - Ben Paselsky (paselsky@battelle.org)
// Version     :
// Copyright   : Copyright (c) 2017 Battelle Memorial Institute. All rights reserved.
// Description : CommandPlugin code for using TmxControl class
//============================================================================

#include "CommandPlugin.h"

namespace CommandPlugin
{

void CommandPlugin::GetTelemetry(string dataType)
{
	TmxControl::pluginlist plugins;
	bool processTelemetry = false;
	set<string> pluginNames;
	map<string, string> *pluginsJSON;
	map<string, string> *pluginsUpdatesJSON;
	map<string, string> *pluginsRemoveJSON;
	string pJSON;
	plugins.push_back("%");
	if (dataType == "List")
	{
		_haveList = _tmxControl.list(plugins);
		if (_haveList)
		{
			//save full json output
			_listJSON = _tmxControl.GetOutput(TmxControlOutputFormat_JSON, false);
			//set flag to process
			processTelemetry = true;
			//set pointers
			pluginsJSON = &_listPluginsJSON;
			pluginsUpdatesJSON = &_listPluginsUpdatesJSON;
			pluginsRemoveJSON = &_listPluginsRemoveJSON;
		}
	}
	else if (dataType == "Status")
	{
		_haveStatus = _tmxControl.status(plugins);
		if (_haveStatus)
		{
			//save full json output
			_statusJSON = _tmxControl.GetOutput(TmxControlOutputFormat_JSON, false);
			//set flag to process
			processTelemetry = true;
			//set pointers
			pluginsJSON = &_statusPluginsJSON;
			pluginsUpdatesJSON = &_statusPluginsUpdatesJSON;
			pluginsRemoveJSON = &_statusPluginsRemoveJSON;
		}
	}
	else if (dataType == "Config")
	{
		_haveConfig = _tmxControl.config(plugins);
		if (_haveConfig)
		{
			//save full json output
			_configJSON = _tmxControl.GetOutput(TmxControlOutputFormat_JSON, false);
			//set flag to process
			processTelemetry = true;
			//set pointers
			pluginsJSON = &_configPluginsJSON;
			pluginsUpdatesJSON = &_configPluginsUpdatesJSON;
			pluginsRemoveJSON = &_configPluginsRemoveJSON;
		}
	}
	else if (dataType == "State")
	{
		_haveState = _tmxControl.state(plugins);
		if (_haveState)
		{
			//save full json output
			_stateJSON = _tmxControl.GetOutput(TmxControlOutputFormat_JSON, false);
			//set flag to process
			processTelemetry = true;
			//set pointers
			pluginsJSON = &_statePluginsJSON;
			pluginsUpdatesJSON = &_statePluginsUpdatesJSON;
			pluginsRemoveJSON = &_statePluginsRemoveJSON;
		}
	}
	else if (dataType == "Messages")
	{
		_haveMessages = _tmxControl.messages(plugins);
		if (_haveMessages)
		{
			//save full json output
			_messagesJSON = _tmxControl.GetOutput(TmxControlOutputFormat_JSON, false);
			//set flag to process
			processTelemetry = true;
			//set pointers
			pluginsJSON = &_messagesPluginsJSON;
			pluginsUpdatesJSON = &_messagesPluginsUpdatesJSON;
			pluginsRemoveJSON = &_messagesPluginsRemoveJSON;
		}
	}
	else if (dataType == "SystemConfig")
	{
		_haveSystemConfig = _tmxControl.system_config(plugins);
		if (_haveSystemConfig)
		{
			//save full json output
			string systemConfigJSON = _tmxControl.GetOutput(TmxControlOutputFormat_JSON, false);
			//get indexes of array
			int firstIndex = systemConfigJSON.find_first_of('[');
			int lastIndex = systemConfigJSON.find_last_of(']');
			if (firstIndex == (int)string::npos || lastIndex == (int)string::npos || firstIndex >= lastIndex || (lastIndex - firstIndex) < 2)
			{
				_systemConfigJSON = "";
				_haveSystemConfig = false;
				return;
			}
			//save json between square brackets
			string cJSON = systemConfigJSON.substr(firstIndex + 1, lastIndex - firstIndex - 1);
			_systemConfigJSON = cJSON;
			//set flag to process
			processTelemetry = true;
			//set pointers
			pluginsJSON = &_systemConfigPluginsJSON;
			pluginsUpdatesJSON = &_systemConfigPluginsUpdatesJSON;
			pluginsRemoveJSON = &_systemConfigPluginsRemoveJSON;
		}
	}
	else
		return;

	if (processTelemetry)
	{
		//clear updates
		pluginsUpdatesJSON->clear();
		//for each plugin
		tmx::message_container_type *output = _tmxControl.GetOutput();
		BOOST_FOREACH(ptree::value_type &plugin, output->get_storage().get_tree())
		{
			stringstream ss;
			ss.clear();
			ss.str(string());
			//get plugin name
			string pluginName = plugin.first;
			//put name in list for later use
			pluginNames.insert(pluginName);
			//get plugin json
			message_tree_type tmpTree;
			if (!plugin.second.empty())
				tmpTree.put_child(plugin.first, plugin.second);
			else
				tmpTree.put(plugin.first, plugin.second.data());

			boost::property_tree::write_json(ss, tmpTree, false);
			pJSON = ss.str();
			boost::algorithm::trim(pJSON);
			//compare json, if not found or different add to updates
			bool found = false;
			if (pluginsJSON->find(pluginName) != pluginsJSON->end())
				found = true;
			if (!found || (*pluginsJSON)[pluginName] != pJSON)
			{
				//read old json into ptree
				ptree oldPlugin;
				if (found)
				{
					istringstream is((*pluginsJSON)[pluginName]);
					read_json(is, oldPlugin);
				}
				//build plugin update json
				bool first = true;
				(*pluginsUpdatesJSON)[pluginName] = "\"";
				(*pluginsUpdatesJSON)[pluginName].append(plugin.first);
				if (dataType == "List" || dataType == "State")
				{
					(*pluginsUpdatesJSON)[pluginName].append("\": {");
					//loop through all name/value pairs of new data and check against old data if updated
					BOOST_FOREACH(ptree::value_type &nvp, plugin.second)
					{
						bool update = false;
						try
						{
							//get old value
							message_path_type pluginKeyPath(pluginName, ATTRIBUTE_PATH_CHARACTER);
							message_path_type keyPath(nvp.first, ATTRIBUTE_PATH_CHARACTER);
							string value = oldPlugin.get_child(pluginKeyPath).get<string>(keyPath);
							if (nvp.second.data() != value)
							{
								//values dont match
								update = true;
							}
						}
						catch (exception ex)
						{
							//old node doesnt exist
							update = true;
						}
						if (update)
						{
							//add this name/value pair to update json
							if (first)
								first = false;
							else
								(*pluginsUpdatesJSON)[pluginName].append(",");
							(*pluginsUpdatesJSON)[pluginName].append("\"");
							(*pluginsUpdatesJSON)[pluginName].append(nvp.first);
							(*pluginsUpdatesJSON)[pluginName].append("\": \"");
							string val = nvp.second.data();
							boost::replace_all(val, "\"", "\\\"");
							(*pluginsUpdatesJSON)[pluginName].append(val);
							(*pluginsUpdatesJSON)[pluginName].append("\"");
						}
					}
					//close plugin brackets
					(*pluginsUpdatesJSON)[pluginName].append("}");
				}
				else if (dataType == "Status")
				{
					//no need to parse since only one value to send
					(*pluginsUpdatesJSON)[pluginName].append("\": \"");
					(*pluginsUpdatesJSON)[pluginName].append(plugin.second.data());
					(*pluginsUpdatesJSON)[pluginName].append("\"");
				}
				else if (dataType == "Config")
				{
					(*pluginsUpdatesJSON)[pluginName].append("\": {");
					//loop through all config variables of new data
					BOOST_FOREACH(ptree::value_type &cfg, plugin.second)
					{
						bool firstNvp = true;
						bool addedConfigVariable = false;
						//loop through all config variable name/value pairs of new data and check against old data if updated
						BOOST_FOREACH(ptree::value_type &nvp, cfg.second)
						{
							bool update = false;
							try
							{
								//get old value
								message_path_type pluginKeyPath(pluginName, ATTRIBUTE_PATH_CHARACTER);
								message_path_type configKeyPath(cfg.first, ATTRIBUTE_PATH_CHARACTER);
								message_path_type keyPath(nvp.first, ATTRIBUTE_PATH_CHARACTER);
								string value = oldPlugin.get_child(pluginKeyPath).get_child(configKeyPath).get<string>(keyPath);
								if (nvp.second.data() != value)
								{
									//values dont match
									update = true;
								}
							}
							catch (exception ex)
							{
								//old node doesnt exist
								update = true;
							}

							if (update)
							{
								if (!addedConfigVariable)
								{
									//this config variable has changed, add it
									if (first)
										first = false;
									else
										(*pluginsUpdatesJSON)[pluginName].append(",");
									(*pluginsUpdatesJSON)[pluginName].append("\"");
									(*pluginsUpdatesJSON)[pluginName].append(cfg.first);
									(*pluginsUpdatesJSON)[pluginName].append("\": {");
									addedConfigVariable = true;
								}
								//add this name/value pair
								if (firstNvp)
									firstNvp = false;
								else
									(*pluginsUpdatesJSON)[pluginName].append(",");
								(*pluginsUpdatesJSON)[pluginName].append("\"");
								(*pluginsUpdatesJSON)[pluginName].append(nvp.first);
								(*pluginsUpdatesJSON)[pluginName].append("\": \"");
								string val = nvp.second.data();
								boost::replace_all(val, "\"", "\\\"");
								(*pluginsUpdatesJSON)[pluginName].append(val);
								(*pluginsUpdatesJSON)[pluginName].append("\"");
							}
						}
						//close brackets if we added this config variable
						if (addedConfigVariable)
							(*pluginsUpdatesJSON)[pluginName].append("}");

					}
					//close plugin brackets
					(*pluginsUpdatesJSON)[pluginName].append("}");
				}
				else if (dataType == "Messages")
				{
					(*pluginsUpdatesJSON)[pluginName].append("\": [");
					//loop through all messages of new data
					BOOST_FOREACH(ptree::value_type &msg, plugin.second)
					{
						bool update = false;
						//get message json
						stringstream msgSs;
						msgSs.clear();
						msgSs.str(string());
						//message_tree_type msgTree;
						//msgTree.put_child("", msg.second);
						boost::property_tree::write_json(msgSs, msg.second, false);
						string msgJSON = msgSs.str();

						if (pJSON.find(msgJSON) == string::npos)
						{
							//values dont match or doesnt exist
							update = true;
						}

						if (update)
						{
							//add full message json to update json
							if (first)
								first = false;
							else
								(*pluginsUpdatesJSON)[pluginName].append(",");
							(*pluginsUpdatesJSON)[pluginName].append(msgJSON);
						}

					}
					//close plugin brackets
					(*pluginsUpdatesJSON)[pluginName].append("]");
				}
				else if (dataType == "SystemConfig")
				{
					//loop through all config variables of new data
					BOOST_FOREACH(ptree::value_type &msg, plugin.second)
					{
						bool update = false;
						//get config json
						stringstream cfgSs;
						cfgSs.clear();
						cfgSs.str(string());
						boost::property_tree::write_json(cfgSs, msg.second, false);
						string cfgSON = cfgSs.str();

						if (pJSON.find(cfgSON) == string::npos)
						{
							//values dont match or doesnt exist
							update = true;
						}

						if (update)
						{
							//add full config json to update json
							if (first)
								first = false;
							else
								(*pluginsUpdatesJSON)[pluginName].append(",");
							(*pluginsUpdatesJSON)[pluginName].append(cfgSON);
						}
					}
				}
			}
			//save json in map
			(*pluginsJSON)[pluginName] = pJSON;
		}
		//clear removes
		pluginsRemoveJSON->clear();
		//remove plugins not in names set and add to remove map
		auto it = pluginsJSON->begin();
		while (it != pluginsJSON->end())
		{
			if (pluginNames.find(it->first) == pluginNames.end())
			{
				(*pluginsRemoveJSON)[it->first] = it->second;
				it = pluginsJSON->erase(it);
			}
			else
				++it;
		}
	}

}

void CommandPlugin::BuildFullTelemetry(string *outputBuffer, string dataType)
{
	ostringstream oss;
	bool processTelemetry = false;
	string output;

	if (dataType == "List" && _haveList)
	{
		output = _listJSON;
		processTelemetry = true;
	}
	else if (dataType == "Status" && _haveStatus)
	{
		output = _statusJSON;
		processTelemetry = true;
	}
	else if (dataType == "Config" && _haveConfig)
	{
		output = _configJSON;
		processTelemetry = true;
	}
	else if (dataType == "State" && _haveState)
	{
		output = _stateJSON;
		processTelemetry = true;
	}
	else if (dataType == "Messages" && _haveMessages)
	{
		output = _messagesJSON;
		processTelemetry = true;
	}
	else if (dataType == "SystemConfig" && _haveSystemConfig)
	{
		output = _systemConfigJSON;
		processTelemetry = true;
	}
	else if (dataType == "Events" && _haveEvents)
	{
		output = _eventsJSON;
		processTelemetry = true;
	}
	else if (dataType == "Heartbeat")
	{
		processTelemetry = true;
	}
	else
		return;

	if (processTelemetry)
	{
		//FILE_LOG(logDEBUG) << "BuildFullTelemetry output: " << output;
		//build  message

		outputBuffer->append("\x02{\"header\":{\"type\":\"Telemetry\",\"subtype\":\"");

		outputBuffer->append(dataType);
		outputBuffer->append("\",\"encoding\":\"jsonstring\",\"timestamp\":\"");
		oss << GetMsTimeSinceEpoch();
		outputBuffer->append(oss.str());
		outputBuffer->append("\",\"flags\":\"0\"},\"payload\":");

		if (dataType == "Events" || dataType == "SystemConfig")
		{
			//add brackets
			outputBuffer->append("[");
			outputBuffer->append(output);
			outputBuffer->append("]");
		}
		else if (dataType == "Heartbeat")
		{
			//empty payload
			outputBuffer->append("{}");
		}
		else
		{
			outputBuffer->append(output);
		}
		//string outputPart = output.substr(0, output.find("TMX UI Vehicle") - 2);
		//outputPart.append("}");
		//_outputBuffer->append(outputPart);

		outputBuffer->append("}\x03");

	}
}

void CommandPlugin::BuildUpdateTelemetry(string *outputBuffer, string dataType)
{
	ostringstream oss;
	bool processTelemetry = false;
	static std::map<string, string> *updates;

	if (dataType == "List" && _haveList)
	{
		if (_listPluginsUpdatesJSON.size() > 0)
		{
			updates = &_listPluginsUpdatesJSON;
			processTelemetry = true;
		}
	}
	else if (dataType == "Status" && _haveStatus)
	{
		if (_statusPluginsUpdatesJSON.size() > 0)
		{
			updates = &_statusPluginsUpdatesJSON;
			processTelemetry = true;
		}
	}
	else if (dataType == "Config" && _haveConfig)
	{
		if (_configPluginsUpdatesJSON.size() > 0)
		{
			updates = &_configPluginsUpdatesJSON;
			processTelemetry = true;
		}
	}
	else if (dataType == "State" && _haveState)
	{
		if (_statePluginsUpdatesJSON.size() > 0)
		{
			updates = &_statePluginsUpdatesJSON;
			processTelemetry = true;
		}
	}
	else if (dataType == "Messages" && _haveMessages)
	{
		if (_messagesPluginsUpdatesJSON.size() > 0)
		{
			updates = &_messagesPluginsUpdatesJSON;
			processTelemetry = true;
		}
	}
	else if (dataType == "Events" && _haveEvents)
	{
		if (_eventsUpdatesJSON.length() > 0)
		{
			processTelemetry = true;
		}
	}
	else if (dataType == "SystemConfig" && _haveSystemConfig)
	{
		if (_systemConfigPluginsUpdatesJSON.size() > 0)
		{
			processTelemetry = true;
		}
	}
	else
		return;

	if (processTelemetry)
	{
		//FILE_LOG(logDEBUG) << "BuildUpdateTelemetry " << dataType;
		//build message

		outputBuffer->append("\x02{\"header\":{\"type\":\"Telemetry\",\"subtype\":\"");

		outputBuffer->append(dataType);
		outputBuffer->append("\",\"encoding\":\"jsonstring\",\"timestamp\":\"");
		oss << GetMsTimeSinceEpoch();
		outputBuffer->append(oss.str());
		if (dataType == "Events")
		{
			//for events only have one chunk of json
			outputBuffer->append("\",\"flags\":\"0\"},\"payload\":[");
			outputBuffer->append(_eventsUpdatesJSON);
			outputBuffer->append("]}\x03");
		}
		else if (dataType == "SystemConfig")
		{
			//for system config only have one chunk of json in first plugin record
			outputBuffer->append("\",\"flags\":\"0\"},\"payload\":[");
			outputBuffer->append(_systemConfigPluginsUpdatesJSON.begin()->second);
			outputBuffer->append("]}\x03");
		}
		else
		{
			outputBuffer->append("\",\"flags\":\"0\"},\"payload\":{");

			//loop through updates
			bool first = true;
			for  (auto it = updates->begin();it != updates->end();it++)
			{
				//FILE_LOG(logDEBUG) << "Update " << it->first << " = " << it->second;
				if (first)
					first = false;
				else
					outputBuffer->append(",");
				//append plugin json
				outputBuffer->append(it->second);
			}

			outputBuffer->append("}}\x03");
		}


	}
}

void CommandPlugin::BuildRemoveTelemetry(string *outputBuffer, string dataType)
{
	ostringstream oss;
	bool processTelemetry = false;
	string output;
	map<string, string> *pluginsRemoveJSON;

	if (dataType == "List" && _haveList && !_listPluginsRemoveJSON.empty())
	{
		pluginsRemoveJSON = &_listPluginsRemoveJSON;
		processTelemetry = true;
	}
	else if (dataType == "Status" && _haveStatus && !_statusPluginsRemoveJSON.empty())
	{
		pluginsRemoveJSON = &_statusPluginsRemoveJSON;
		processTelemetry = true;
	}
	else if (dataType == "Config" && _haveConfig && !_configPluginsRemoveJSON.empty())
	{
		pluginsRemoveJSON = &_configPluginsRemoveJSON;
		processTelemetry = true;
	}
	else if (dataType == "State" && _haveState && !_statePluginsRemoveJSON.empty())
	{
		pluginsRemoveJSON = &_statePluginsRemoveJSON;
		processTelemetry = true;
	}
	else if (dataType == "Messages" && _haveMessages && !_messagesPluginsRemoveJSON.empty())
	{
		pluginsRemoveJSON = &_messagesPluginsRemoveJSON;
		processTelemetry = true;
	}
	else
		return;

	if (processTelemetry)
	{
		//FILE_LOG(logDEBUG) << "BuildRemoveTelemetry output: " << output;
		//build  message

		outputBuffer->append("\x02{\"header\":{\"type\":\"Telemetry\",\"subtype\":\"");
		outputBuffer->append("Remove_");
		outputBuffer->append(dataType);
		outputBuffer->append("\",\"encoding\":\"jsonstring\",\"timestamp\":\"");
		oss << GetMsTimeSinceEpoch();
		outputBuffer->append(oss.str());
		outputBuffer->append("\",\"flags\":\"0\"},\"payload\":{");
		//loop through updates
		bool first = true;
		for  (auto it = pluginsRemoveJSON->begin();it != pluginsRemoveJSON->end();it++)
		{
			//FILE_LOG(logDEBUG) << "pluginsRemoveJSON " << it->first << " = " << it->second;
			//append plugin json
			int firstBracket = it->second.find_first_of('{');
			int lastBracket = it->second.find_last_of('}');
			if (lastBracket - firstBracket > 1)
			{
				if (first)
					first = false;
				else
					outputBuffer->append(",");
				outputBuffer->append(it->second.substr(firstBracket + 1, lastBracket - firstBracket - 1));
			}
		}

		outputBuffer->append("}}\x03");

	}
}

void CommandPlugin::GetEventTelemetry()
{
	TmxControl::pluginlist plugins;
	plugins.push_back("%");
	_tmxControl.ClearOptions();
	if (_lastEventTime != "")
		_tmxControl.SetOption("eventTime", _lastEventTime);
	else
		_tmxControl.SetOption("rowLimit", std::to_string(_eventRowLimit));
	_haveEvents = _tmxControl.events(plugins);

	if (_haveEvents)
	{
		//get json output
		string eJSON = _tmxControl.GetOutput(TmxControlOutputFormat_JSON, false);
		//return if no new events
		if (eJSON == "")
		{
			_eventsUpdatesJSON = "";
			return;
		}
		FILE_LOG(logDEBUG) << "Events: " << eJSON;
		//extract last timestamp
		int lastIndex = eJSON.find_last_of('\"');
		if (lastIndex == (int)string::npos)
		{
			_eventsUpdatesJSON = "";
			return;
		}
		int firstIndex;
		for (firstIndex = lastIndex - 1;firstIndex >= 0 && eJSON[firstIndex] != '\"';firstIndex--);
		if (firstIndex < 0 || lastIndex - firstIndex < 2)
		{
			_eventsUpdatesJSON = "";
			return;
		}
		_lastEventTime = eJSON.substr(firstIndex + 1, lastIndex - firstIndex - 1);
		//get indexes of array
		firstIndex = eJSON.find_first_of('[');
		lastIndex = eJSON.find_last_of(']');
		if (firstIndex == (int)string::npos || lastIndex == (int)string::npos || firstIndex >= lastIndex)
		{
			_eventsUpdatesJSON = "";
			return;
		}
		//count number of lines in the new JSON string
		int eventCount = 1;
		int stringPosition = eJSON.find("},{", firstIndex);
		while (stringPosition < eJSON.length() && stringPosition != string::npos)
		{
			eventCount++;
			stringPosition += 3;
			stringPosition = eJSON.find("},{", stringPosition);
		}
		//FILE_LOG(logDEBUG) << "GetEventTelemetry: new events: " << eventCount;
		if (_eventsJSON.length() >= 0)
		{
			//save update json without brackets if already have initial full json
			_eventsUpdatesJSON = eJSON.substr(firstIndex + 1, lastIndex - firstIndex - 1);
			if(_eventsJSON.length() > 0){
				_eventsJSON.append(",");
			}
		}
		//add JSON to next JSON buffer
		if (_eventsFullCount >= _eventRowLimit)
		{
			if (_eventsNextFullJSON.length() > 0)
				_eventsNextFullJSON.append(",");
			_eventsNextFullJSON.append(eJSON.substr(firstIndex + 1, lastIndex - firstIndex - 1));
			_eventsNextFullCount += eventCount;
		}
		if (_eventsFullCount >= (_eventRowLimit * 2))
		{
			//copy next JSON buffer into full JSON buffer
			_eventsJSON = _eventsNextFullJSON;
			_eventsNextFullJSON = "";
			_eventsFullCount =_eventsNextFullCount;
			_eventsNextFullCount = 0;
		}
		else
		{
			//append to full json without brackets
			_eventsJSON.append(eJSON.substr(firstIndex + 1, lastIndex - firstIndex - 1));
			_eventsFullCount += eventCount;
		}

		//FILE_LOG(logDEBUG) << "GetEventTelemetry: full events: " << _eventsFullCount << ", next events: " << _eventsNextFullCount;
	}

}

void CommandPlugin::BuildCommandResponse(string *outputBuffer, string id, string command, string status, string reason, std::map<string, string> &data, std::map<string, string> &arrayData)
{
	ostringstream oss;
	//build message
	outputBuffer->append("\x02{\"header\":{\"type\":\"Command\",\"subtype\":\"Execute\",\"encoding\":\"jsonstring\",\"timestamp\":\"");
	oss << GetMsTimeSinceEpoch();
	outputBuffer->append(oss.str());
	outputBuffer->append("\",\"flags\":\"0\"},\"payload\":{");
	outputBuffer->append("\"id\":\"");
	outputBuffer->append(id);
	outputBuffer->append("\",\"status\":\"");
	outputBuffer->append(status);
	outputBuffer->append("\",\"command\":\"");
	outputBuffer->append(command);
	outputBuffer->append("\",\"reason\":\"");
	outputBuffer->append(reason);
	for (auto it = data.begin();it != data.end();++it)
	{
		outputBuffer->append("\",\"");
		outputBuffer->append(it->first);
		outputBuffer->append("\":\"");
		outputBuffer->append(it->second);
	}
	outputBuffer->append("\"");
	for (auto it = arrayData.begin();it != arrayData.end();++it)
	{
		outputBuffer->append(",\"");
		outputBuffer->append(it->first);
		outputBuffer->append("\":");
		outputBuffer->append(it->second);
	}
	outputBuffer->append("}}\x03");
}

void CommandPlugin::SendData(string *outputBuffer, struct lws *wsi)
{
	//check for empty buffer
	if (outputBuffer->length() == 0)
		return;

	//FILE_LOG(logDEBUG) << "SendData";
	//encode to base64 and send

	//send a chunk of data if outputBuffer is too big
	string pluginList64;
	if (outputBuffer->length() > MAX_SEND_BYTES)
	{
		//send chunk and save remainder
		pluginList64 = Base64::Encode((const unsigned char*)outputBuffer->substr(0, MAX_SEND_BYTES).c_str(), MAX_SEND_BYTES);
		*outputBuffer = outputBuffer->substr(MAX_SEND_BYTES);
	}
	else
	{
		//send the whole thing and clear outputBuffer
		pluginList64 = Base64::Encode((const unsigned char*)outputBuffer->c_str(), outputBuffer->length());
		outputBuffer->clear();
	}

	unsigned char writeBuffer[LWS_SEND_BUFFER_PRE_PADDING + pluginList64.length() + LWS_SEND_BUFFER_POST_PADDING];
	memcpy(&writeBuffer[LWS_SEND_BUFFER_PRE_PADDING], pluginList64.c_str(), pluginList64.length());

	//check if send pipe is choked
	int chokeCount = 0;
	while (lws_send_pipe_choked(wsi) && chokeCount < 4)
	{
		this_thread::sleep_for(chrono::milliseconds(5));
		chokeCount++;
	}
	if (chokeCount < 4)
		lws_write(wsi, &writeBuffer[LWS_SEND_BUFFER_PRE_PADDING], pluginList64.length(), LWS_WRITE_TEXT);


	//lws_write(wsi, &writeBuffer[LWS_SEND_BUFFER_PRE_PADDING], pluginList64.length(), LWS_WRITE_TEXT);

}

} /* namespace CommandPlugin */
