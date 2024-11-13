//============================================================================
// Name        : CommandPlugin.cpp
// Author      : Battelle Memorial Institute - Ben Paselsky (paselsky@battelle.org)
// Version     :
// Copyright   : Copyright (c) 2017 Battelle Memorial Institute. All rights reserved.
// Description : Plugin that listens for websocket connections from the TMX admin portal
//				 and processes commands
//============================================================================

#include "CommandPlugin.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

namespace CommandPlugin
{

const char * CommandPlugin::_httpParamNames[4] = {
		"text",
		"send",
		"file",
		"upload",
};

std::atomic<uint64_t> CommandPlugin::_eventRowLimit{500};
string CommandPlugin::_downloadPath = "/var/www/download";
mutex CommandPlugin::_configLock;
string CommandPlugin::_databaseAddress = "127.0.0.1";
string CommandPlugin::_databasePort = "3306";
uint64_t CommandPlugin::_updateIntervalMS = 1000;
uint64_t CommandPlugin::_heartbeatIntervalMS = 30000;
uint64_t CommandPlugin::_lastPluginsUpdateTimeMS = 0;
uint64_t CommandPlugin::_nextSession = 1;
TmxControl CommandPlugin::_tmxControl;
uint64_t CommandPlugin::_connectionCount = 0;
bool CommandPlugin::_haveList = false;
bool CommandPlugin::_haveConfig = false;
bool CommandPlugin::_haveStatus = false;
bool CommandPlugin::_haveState = false;
bool CommandPlugin::_haveMessages = false;
bool CommandPlugin::_haveSystemConfig = false;
bool CommandPlugin::_haveEvents = false;
tmx::message_container_type CommandPlugin::_listJSON;
string CommandPlugin::_configJSON = "";
string CommandPlugin::_statusJSON = "";
string CommandPlugin::_stateJSON = "";
string CommandPlugin::_messagesJSON = "";
string CommandPlugin::_systemConfigJSON = "";
string CommandPlugin::_eventsJSON = "";
string CommandPlugin::_lastEventTime = "";
std::map<string, string> CommandPlugin::_listPluginsJSON;
std::map<string, string> CommandPlugin::_configPluginsJSON;
std::map<string, string> CommandPlugin::_statusPluginsJSON;
std::map<string, string> CommandPlugin::_statePluginsJSON;
std::map<string, string> CommandPlugin::_messagesPluginsJSON;
std::map<string, string> CommandPlugin::_systemConfigPluginsJSON;
std::map<string, tmx::message_container_type> CommandPlugin::_listPluginsUpdatesJSON;
std::map<string, string> CommandPlugin::_configPluginsUpdatesJSON;
std::map<string, string> CommandPlugin::_statusPluginsUpdatesJSON;
std::map<string, string> CommandPlugin::_statePluginsUpdatesJSON;
std::map<string, string> CommandPlugin::_messagesPluginsUpdatesJSON;
std::map<string, string> CommandPlugin::_systemConfigPluginsUpdatesJSON;
std::map<string, string> CommandPlugin::_listPluginsRemoveJSON;
std::map<string, string> CommandPlugin::_configPluginsRemoveJSON;
std::map<string, string> CommandPlugin::_statusPluginsRemoveJSON;
std::map<string, string> CommandPlugin::_statePluginsRemoveJSON;
std::map<string, string> CommandPlugin::_messagesPluginsRemoveJSON;
std::map<string, string> CommandPlugin::_systemConfigPluginsRemoveJSON;
string CommandPlugin::_eventsUpdatesJSON = "";
string CommandPlugin::_eventsNextFullJSON = "";
uint64_t CommandPlugin::_eventsFullCount = 0;
uint64_t CommandPlugin::_eventsNextFullCount = 0;

std::map<string, CommandPlugin::UploadData> CommandPlugin::_uploadRequests;


/**
 * Construct a new CommandPlugin with the given name.
 *
 * @param name The name to give the plugin for identification purposes
 */
CommandPlugin::CommandPlugin(string name) : PluginClient(name)
{
	//set tmxcontrol connection url
	_tmxControl.SetConnectionUrl(string("tcp://" + _databaseAddress + ":" + _databasePort));
	_tmxControl.DisablePermissionCheck();

	// Add a message filter and handler for each message this plugin wants to receive.

	// Subscribe to all messages specified by the filters above.

}

CommandPlugin::~CommandPlugin()
{
}

void CommandPlugin::OnConfigChanged(const char *key, const char *value)
{
	PluginClient::OnConfigChanged(key, value);
	UpdateConfigSettings();
}

void CommandPlugin::OnStateChange(IvpPluginState state)
{
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
	}
}

void CommandPlugin::UpdateConfigSettings()
{
	GetConfigValue<uint64_t>("SleepMS", _sleepMS);
	GetConfigValue<bool>("SSLEnabled", _sslEnabled);
	{
		lock_guard<mutex> lock(_configLock);
		GetConfigValue<string>("SSLPath", _sslPath);
		GetConfigValue<string>("DownloadPath", _downloadPath);
	}
	GetConfigValue<uint64_t>("EventRowLimit", _eventRowLimit);

	//PLOG(logDEBUG) << "    Config data - SleepMS: " << SleepMS ;

	_newConfigValues = true;
}

uint64_t CommandPlugin::GetMsTimeSinceEpoch()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t) ((double) (tv.tv_sec) * 1000
			+ (double) (tv.tv_usec) / 1000);
}

int CommandPlugin::FileUploadCB(void *data, const char *name, const char *filename,
	       char *buf, int len, enum lws_spa_fileupload_states state)
{
	struct PerSessionDataHTTP *pss =
			(struct PerSessionDataHTTP *)data;
	string outdir;
	{
		lock_guard<mutex> lock(_configLock);
		outdir = _downloadPath;
	}
	boost::filesystem::path outpath(outdir);
	string fromFile;
	string toFile;
	std::map<string, string> objectData;
	std::map<string, string> arrayData;
	uint64_t fileLength;
#if !defined(LWS_WITH_ESP32)
	int n = 0;
#endif
	//FILE_LOG(logDEBUG) << "CommandPlugin::FileUploadCB: state = " << (int)state << ", len = " << len;
	switch (state) {
	case LWS_UFS_OPEN:
		strncpy(pss->filename, filename, sizeof(pss->filename) - 1);
		if (_uploadRequests.find(pss->filename) == _uploadRequests.end())
		{
			_uploadRequests[pss->filename].message = "Permission denied";
			return 1;
		}
		FILE_LOG(logDEBUG) << "CommandPlugin::FileUploadCB: Found request";
		try
		{
			FILE_LOG(logDEBUG) << "CommandPlugin::FileUploadCB: outpath = " << outpath.c_str();
			if (!boost::filesystem::exists(outpath))
				boost::filesystem::create_directory(outpath);
		}
		catch (exception & ex)
		{
			FILE_LOG(logERROR) << "CommandPlugin::FileUploadCB: Failed to create download folder";
			_uploadRequests[pss->filename].message = "Failed to create download folder";
			return 1;
		}
		FILE_LOG(logDEBUG) << "CommandPlugin::FileUploadCB: file name = " << pss->filename;
		outdir.append("/");
		outdir.append(pss->filename);
#if !defined(LWS_WITH_ESP32)
		pss->fd = (lws_filefd_type)(long long)open(outdir.c_str(),
				O_CREAT | O_TRUNC | O_RDWR, 0600);
		if (pss->fd < 0)
		{
			FILE_LOG(logERROR) << "CommandPlugin::FileUploadCB: Failed to create or open file";
			_uploadRequests[pss->filename].message = "Failed to create or open file";
			return 1;
		}
		_uploadRequests[pss->filename].uploading = true;
#endif
		break;
	case LWS_UFS_FINAL_CONTENT:
	case LWS_UFS_CONTENT:
		//check if connection dropped
		if (_uploadRequests[pss->filename].outputbuffer == NULL)
		{
			FILE_LOG(logDEBUG) << "CommandPlugin::FileUploadCB: skip content, connection dropped";
			_uploadRequests[pss->filename].message = "Connection dropped";
			if (pss->fd >= 0)
				close((int)(long long)pss->fd);
			if (state == LWS_UFS_FINAL_CONTENT)
				_uploadRequests[pss->filename].uploading = false;
			pss->fd = LWS_INVALID_FILE;
			return 1;
		}
		if (len && pss->fd >= 0)
		{
			pss->file_length += len;
			fileLength = pss->file_length;
			//FILE_LOG(logDEBUG) << "CommandPlugin::FileUploadCB: file_length = " << fileLength << ", fileSize = " << _uploadRequests[pss->filename].fileSize;
			//check if need to send percent update
			if (_uploadRequests[pss->filename].fileSize > 0 && (fileLength * 100 / _uploadRequests[pss->filename].fileSize) >  pss->last_percent_sent)
			{
				pss->last_percent_sent = fileLength * 100 / _uploadRequests[pss->filename].fileSize;
				objectData["state"] = "sending";
				objectData["percent"] = std::to_string(pss->last_percent_sent);
				//FILE_LOG(logDEBUG) << "CommandPlugin::FileUploadCB: last_percent_sent = " << objectData["percent"];
				BuildCommandResponse(_uploadRequests[pss->filename].outputbuffer, _uploadRequests[pss->filename].requestId, "uploadfile", "success", "", objectData, arrayData);
			}
			/* if the file length is too big, drop it */
			if (pss->file_length > 100000000)
			{
				_uploadRequests[pss->filename].message = "File is too large";
				if (pss->fd >= 0)
					close((int)(long long)pss->fd);
				pss->fd = LWS_INVALID_FILE;
				return 1;
			}
#if !defined(LWS_WITH_ESP32)
			n = write((int)(long long)pss->fd, buf, len);
			//FILE_LOG(logDEBUG) << "CommandPlugin::FileUploadCB: bytes to write = " << len << ", bytes written = " << n;
#else
			//FILE_LOG(logDEBUG) << "CommandPlugin::FileUploadCB: bytes received = " << len;
#endif
		}
		if (state == LWS_UFS_CONTENT)
			break;
#if !defined(LWS_WITH_ESP32)
		if (pss->fd >= 0)
		{
			close((int)(long long)pss->fd);
			//move file if required
			if (_uploadRequests[pss->filename].destinationFileName != "")
			{
				fromFile = outdir;
				fromFile.append("/");
				fromFile.append(pss->filename);
				if (_uploadRequests[pss->filename].destinationPath == "")
				{
					toFile = outdir;
				}
				else
				{
					toFile = _uploadRequests[pss->filename].destinationPath;
					//create folder if does not exist
					try
					{
						if (!boost::filesystem::exists(toFile))
							boost::filesystem::create_directory(toFile);
					}
					catch (exception & ex)
					{
						FILE_LOG(logERROR) << "CommandPlugin::FileUploadCB: Failed to create destination folder";
						_uploadRequests[pss->filename].message = "Failed to create destination folder";
						return 1;
					}
				}
				toFile.append("/");
				toFile.append(_uploadRequests[pss->filename].destinationFileName);
				try
				{
					boost::filesystem::copy_file (fromFile, toFile, copy_option::overwrite_if_exists);
				}
				catch (exception & ex)
				{
					FILE_LOG(logERROR) << "CommandPlugin::FileUploadCB: Failed to copy file to destination folder: " << toFile;
					_uploadRequests[pss->filename].message = "Failed to copy file to destination folder.";
					try
					{
						boost::filesystem::remove(fromFile);
						FILE_LOG(logERROR) << "CommandPlugin::FileUploadCB: Failed to delete download file: " << fromFile;
						_uploadRequests[pss->filename].message.append(" Failed to delete download file.");
					}
					catch (exception & ex2)
					{
					}
					return 1;
				}
				try
				{
					boost::filesystem::remove(fromFile);
				}
				catch (exception & ex)
				{
					FILE_LOG(logERROR) << "CommandPlugin::FileUploadCB: Failed to delete download file.";
					_uploadRequests[pss->filename].message = "Failed to delete download file.";
					return 1;
				}
			}
		}
		pss->fd = LWS_INVALID_FILE;

#endif
		break;
	}

	return 0;
}


int CommandPlugin::WSCallbackHTTP(
		struct lws *wsi,
		enum lws_callback_reasons reason,
		void *user,
		void *in, size_t len)
{
	//FILE_LOG(logDEBUG) << "WSCallbackHTTP reason: " << (int)reason;
	//char response[] = "Hello!";

	struct PerSessionDataHTTP *pss = (struct PerSessionDataHTTP *)user;
	unsigned char *buffer;
	unsigned char *p, *start, *end;
	int n;
	std::map<string, string> data;
	std::map<string, string> arrayData;

	switch (reason)
	{
	case LWS_CALLBACK_ESTABLISHED:
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		break;
	case LWS_CALLBACK_HTTP:
		//lws_write(wsi, (unsigned char*)response, strlen(response), LWS_WRITE_HTTP);
		break;
	case LWS_CALLBACK_HTTP_BODY:
		/* create the POST argument parser if not already existing */
		if (!pss->spa) {
			//FILE_LOG(logDEBUG) << "WSCallbackHTTP LWS_CALLBACK_HTTP_BODY create";
			pss->spa = lws_spa_create(wsi, _httpParamNames,
					LWS_ARRAY_SIZE(_httpParamNames), 1024,
					FileUploadCB, pss);
			if (!pss->spa)
				return -1;

			pss->filename[0] = '\0';
			pss->file_length = 0;
			pss->last_percent_sent = 0;
		}
		else
		{
			if (_uploadRequests.find(pss->filename) != _uploadRequests.end() && _uploadRequests[pss->filename].outputbuffer == NULL)
			{
				FILE_LOG(logDEBUG) << "WSCallbackHTTP LWS_CALLBACK_HTTP_BODY drop protocol";
				return -1;
			}
		}

		//FILE_LOG(logDEBUG) << "WSCallbackHTTP process data len: " << (int)len;
		/* let it parse the POST data */
		if (lws_spa_process(pss->spa, (const char *)in, (int)len))
			return -1;
		//FILE_LOG(logDEBUG) << "WSCallbackHTTP LWS_CALLBACK_HTTP_BODY processed";
		break;

	case LWS_CALLBACK_HTTP_BODY_COMPLETION:
		//lwsl_debug("LWS_CALLBACK_HTTP_BODY_COMPLETION\n");
		/* call to inform no more payload data coming */
		lws_spa_finalize(pss->spa);

		p = (unsigned char *)pss->result + LWS_PRE;
		end = p + sizeof(pss->result) - LWS_PRE - 1;
		p += sprintf((char *)p,
			"<html><body><h1>Form results (after urldecoding)</h1>"
			"<table><tr><td>Name</td><td>Length</td><td>Value</td></tr>");

		for (n = 0; n < (int)LWS_ARRAY_SIZE(_httpParamNames); n++) {
			if (!lws_spa_get_string(pss->spa, n))
				p += lws_snprintf((char *)p, end - p,
				    "<tr><td><b>%s</b></td><td>0</td><td>NULL</td></tr>",
					_httpParamNames[n]);
			else
				p += lws_snprintf((char *)p, end - p,
				    "<tr><td><b>%s</b></td><td>%d</td><td>%s</td></tr>",
					_httpParamNames[n],
				    lws_spa_get_length(pss->spa, n),
				    lws_spa_get_string(pss->spa, n));
		}

		p += lws_snprintf((char *)p, end - p, "</table><br><b>filename:</b> %s, <b>length</b> %ld",
				pss->filename, pss->file_length);

		p += lws_snprintf((char *)p, end - p, "</body></html>");
		pss->result_len = lws_ptr_diff(p, pss->result + LWS_PRE);

		n = LWS_PRE + 1024;
		buffer = (unsigned char*)malloc(n);
		p = buffer + LWS_PRE;
		start = p;
		end = p + n - LWS_PRE - 1;

		if (lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p, end))
		{
			free(buffer);
			return 1;
		}

		if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
				(unsigned char *)"text/html", 9, &p, end))
		{
			free(buffer);
			return 1;
		}
		if (lws_add_http_header_content_length(wsi, pss->result_len, &p, end))
		{
			free(buffer);
			return 1;
		}
		if (lws_finalize_http_header(wsi, &p, end))
		{
			free(buffer);
			return 1;
		}

		n = lws_write(wsi, start, p - start, LWS_WRITE_HTTP_HEADERS);
		if (n < 0)
		{
			free(buffer);
			return 1;
		}
		free(buffer);

		lws_callback_on_writable(wsi);
		break;

	case LWS_CALLBACK_HTTP_WRITEABLE:
		if (!pss->result_len)
			break;
		//lwsl_debug("LWS_CALLBACK_HTTP_WRITEABLE: sending %d\n",
		//	   pss->result_len);
		n = lws_write(wsi, (unsigned char *)pss->result + LWS_PRE,
			      pss->result_len, LWS_WRITE_HTTP);
		if (n < 0)
			return 1;
		if (lws_http_transaction_completed(wsi))
			return -1;
		break;

	case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
		/* called when our wsi user_space is going to be destroyed */
		if (pss->spa) {
			//send response
			if (_uploadRequests.find(pss->filename) != _uploadRequests.end())
			{
				if (_uploadRequests[pss->filename].outputbuffer != NULL)
				{
					data["state"] = "upload";
					if (_uploadRequests[pss->filename].message == "")
						BuildCommandResponse(_uploadRequests[pss->filename].outputbuffer, _uploadRequests[pss->filename].requestId, "uploadfile", "success", "", data, arrayData);
					else
						BuildCommandResponse(_uploadRequests[pss->filename].outputbuffer, _uploadRequests[pss->filename].requestId, "uploadfile", "failed", _uploadRequests[pss->filename].message, data, arrayData);
				}
				//remove request from list
				FILE_LOG(logDEBUG) << "WSCallbackHTTP LWS_CALLBACK_HTTP_DROP_PROTOCOL erase request";
				_uploadRequests.erase(pss->filename);
			}
			lws_spa_destroy(pss->spa);
			pss->spa = NULL;
		}
		break;

	default:
		break;
	}

	return 0;
}


int CommandPlugin::WSCallbackBASE64(
		struct lws *wsi,
		enum lws_callback_reasons reason,
		void *user,
		void *in, size_t len)
{
	//if (!user)
	//	return 0;
	struct PerSessionDataBASE64 *psdata = (struct PerSessionDataBASE64 *)user;
	//FILE_LOG(logDEBUG) << "WSCallbackBASE64 reason: " << (int)reason;
	switch (reason)
	{
	case LWS_CALLBACK_ESTABLISHED:
		//FILE_LOG(logDEBUG) << "WSCallbackBASE64 initial lists";
		{
			_connectionCount++;
			//initialize per session data
			psdata->bufferLength = 0;
			psdata->lastUpdateSendTimeMS = GetMsTimeSinceEpoch();
			psdata->session = _nextSession++;
			psdata->sendFullTelemetry = true;
			psdata->outputbuffer = new string("");
			psdata->lastEventId = 0;
			psdata->authorized = false;
			psdata->authorizationLevel = AuthorizationLevels::ReadOnly;
			psdata->lastHeartbeatSendTimeMS = GetMsTimeSinceEpoch();
		}
		break;
	case LWS_CALLBACK_CLOSED:
		{
			_connectionCount--;
			//clear all data if connection count goes to 0
			if (_connectionCount == 0)
			{
				_lastPluginsUpdateTimeMS = 0;
				_haveList = false;
				_haveConfig = false;
				_haveStatus = false;
				_haveState = false;
				_haveMessages = false;
				_haveSystemConfig = false;
				_haveEvents = false;
				_listJSON.get_storage().get_tree().empty();
				_configJSON = "";
				_statusJSON = "";
				_stateJSON = "";
				_messagesJSON = "";
				_systemConfigJSON = "";
				_eventsJSON = "";
				_lastEventTime = "";
				_listPluginsJSON.clear();
				_configPluginsJSON.clear();
				_statusPluginsJSON.clear();
				_statePluginsJSON.clear();
				_messagesPluginsJSON.clear();
				_systemConfigPluginsJSON.clear();
				_listPluginsUpdatesJSON.clear();
				_configPluginsUpdatesJSON.clear();
				_statusPluginsUpdatesJSON.clear();
				_statePluginsUpdatesJSON.clear();
				_messagesPluginsUpdatesJSON.clear();
				_systemConfigPluginsUpdatesJSON.clear();
				_listPluginsRemoveJSON.clear();
				_configPluginsRemoveJSON.clear();
				_statusPluginsRemoveJSON.clear();
				_statePluginsRemoveJSON.clear();
				_messagesPluginsRemoveJSON.clear();
				_systemConfigPluginsRemoveJSON.clear();
				_eventsUpdatesJSON = "";
			}
			//delete allocated buffer and set buffer to NULL for uploading requests with this buffer, otherwise delete request
			for (auto it = _uploadRequests.begin();it != _uploadRequests.end();)
			{
				if (it->second.outputbuffer == psdata->outputbuffer && !it->second.uploading)
				{
					it = _uploadRequests.erase(it);
				}
				else
				{
					it->second.outputbuffer = NULL;
					it++;
				}
			}
			delete psdata->outputbuffer;
		}
		break;
	case LWS_CALLBACK_RECEIVE:
		//FILE_LOG(logDEBUG) << "WSCallbackBASE64 RECEIVE len: " << len;
		{
			//decode message
			string message = Base64::Decode(string((const char *)in, len));
			const char *msgBytes = message.c_str();
			for (unsigned int i = 0;i<message.length();i++)
			{
				if (msgBytes[i] == 0x02)
				{
					//start of message
					//reset count
					psdata->bufferLength = 0;
				}
				else if (msgBytes[i] == 0x03)
				{
					//end of message
					if (psdata->bufferLength > 0)
					{
						//process message
						if (psdata->bufferLength >= READ_BUFFER_SIZE)
						{
							//buffer is full, cant process
							FILE_LOG(logDEBUG) << "WSCallbackBASE64 incoming message is too large";
						}
						else
						{
							//terminate string
							psdata->buffer[psdata->bufferLength] = '\0';
							//process message
							FILE_LOG(logDEBUG) << "WSCallbackBASE64 processing message '" << psdata->buffer << "'";

							try
							{
								// Read the JSON into a boost property tree.
								ptree pt;
								ptree header;
								ptree payload;
								istringstream is(psdata->buffer);
								read_json(is, pt);
								//get header info
								header = pt.get_child("header");
								string msgType = header.get<string>("type");
								string msgSubtype = header.get<string>("subtype");
								if (msgType == "Command" && msgSubtype == "Execute")
								{
									//process command
									payload = pt.get_child("payload");
									//ptree authorization = payload.get_child("authorization");
									//string user = authorization.get<string>("user");
									//string password = authorization.get<string>("password");
									string command = payload.get<string>("command");
									FILE_LOG(logDEBUG) << "WSCallbackBASE64 processing Execute command " << command;
									string id = payload.get<string>("id");
									map<string, string> argsList;
									try
									{
										//get arg list
										ptree args = payload.get_child("args");
										BOOST_FOREACH(ptree::value_type &arg, args)
										{
											argsList[arg.first] = arg.second.data();
										}
									}
									catch (const exception &argsEx)
									{
										//no args
										FILE_LOG(logDEBUG) << "WSCallbackBASE64 process command error: no arguments. " << argsEx.what();
									}

									//check authorization
									if (psdata->authorized)
									{
										//process command if authorized at correct level
										if (command == "enable" && psdata->authorizationLevel >= AuthorizationLevels::ApplicationAdministrator)
										{
											if (argsList.find("plugin") != argsList.end())
											{
												TmxControl::pluginlist plugins;
												plugins.push_back(argsList["plugin"]);
												bool rc = _tmxControl.enable(plugins);
												if (rc)
													FILE_LOG(logDEBUG) << "WSCallbackBASE64 enable " << argsList["plugin"] << " success";
												else
													FILE_LOG(logDEBUG) << "WSCallbackBASE64 enable " << argsList["plugin"] << " failed";
											}
										}
										else if (command == "disable" && psdata->authorizationLevel >= AuthorizationLevels::ApplicationAdministrator)
										{
											if (argsList.find("plugin") != argsList.end())
											{
												TmxControl::pluginlist plugins;
												plugins.push_back(argsList["plugin"]);
												bool rc = _tmxControl.disable(plugins);
												if (rc)
													FILE_LOG(logDEBUG) << "WSCallbackBASE64 disable " << argsList["plugin"] << " success";
												else
													FILE_LOG(logDEBUG) << "WSCallbackBASE64 disable " << argsList["plugin"] << " failed";
											}
										}
										else if (command == "set" && psdata->authorizationLevel >= AuthorizationLevels::ApplicationAdministrator)
										{
											if (argsList.find("plugin") != argsList.end())
											{
												if (argsList.find("key") != argsList.end() && argsList.find("value") != argsList.end())
												{
													TmxControl::pluginlist plugins;
													bool rc;
													plugins.push_back(argsList["plugin"]);
													_tmxControl.ClearOptions();
													if (argsList["key"] == "maxMessageInterval")
													{
														//set max message interval
														_tmxControl.SetOption("max-message-interval", argsList["value"]);
														rc = _tmxControl.max_message_interval(plugins);
													}
													else
													{
														//set plugin or system config parameter
														_tmxControl.SetOption("key", argsList["key"]);
														_tmxControl.SetOption("value", argsList["value"]);
														if (argsList.find("defaultValue") != argsList.end())
															_tmxControl.SetOption("defaultValue", argsList["defaultValue"]);
														else
															_tmxControl.SetOption("defaultValue", argsList[""]);
														if (argsList.find("description") != argsList.end())
															_tmxControl.SetOption("description", argsList["description"]);
														else
															_tmxControl.SetOption("description", argsList["Added by CommandPlugin"]);
														if (argsList["plugin"] == "SYSTEMCONFIG")
															rc = _tmxControl.set_system(plugins);
														else
															rc = _tmxControl.set(plugins);

													}
													//check if we are setting a system config parameter
													if (rc)
														FILE_LOG(logDEBUG) << "WSCallbackBASE64 set " << argsList["plugin"] << ": " << argsList["key"] << "=" << argsList["value"] << " success";
													else
														FILE_LOG(logDEBUG) << "WSCallbackBASE64 set " << argsList["plugin"] << ": " << argsList["key"] << "=" << argsList["value"] << " failed";
												}
											}
										}
										else if (command == "logout")
										{
											//set per session data
											psdata->bufferLength = 0;
											psdata->lastUpdateSendTimeMS = GetMsTimeSinceEpoch();
											psdata->session = _nextSession++;
											psdata->sendFullTelemetry = true;
											psdata->outputbuffer->clear();
											psdata->lastEventId = 0;
											psdata->authorized = false;
											psdata->authorizationLevel = AuthorizationLevels::ReadOnly;

											//send response
											std::map<string, string> data;
											BuildCommandResponse(psdata->outputbuffer, id, command, "success", "", data, data);

											FILE_LOG(logDEBUG) << "WSCallbackBASE64 logout";
										}
										else if (command == "clearlog" && psdata->authorizationLevel >= AuthorizationLevels::SystemAdministrator)
										{
											TmxControl::pluginlist plugins;
											plugins.push_back("%");
											bool rc = _tmxControl.clear_event_log(plugins);
											std::map<string, string> data;
											if (rc)
											{
												FILE_LOG(logDEBUG) << "WSCallbackBASE64 clear_event_log success";
												//clear events data
												_eventsJSON = "";
												_eventsUpdatesJSON = "";
												//send response
												BuildCommandResponse(psdata->outputbuffer, id, command, "success", "", data, data);

											}
											else
											{
												FILE_LOG(logDEBUG) << "WSCallbackBASE64 clear_event_log failed";
												BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "Clear event log failed", data, data);
											}
										}
										else if (command == "userlist" && psdata->authorizationLevel >= AuthorizationLevels::SystemAdministrator)
										{
											std::map<string, string> data;
											std::map<string, string> arrayData;
											bool rc = _tmxControl.all_users_info();
											if (rc)
											{
												//get json output
												string uJSON = _tmxControl.GetOutput(TmxControlOutputFormat_JSON, false);
												string outputArray = "";
												string username;
												string accesslevel;
												bool firstUser = true;
												if (uJSON != "")
												{
													outputArray.append("[");
													tmx::message_container_type *output = _tmxControl.GetOutput();
													BOOST_FOREACH(ptree::value_type &userInfo, output->get_storage().get_tree())
													{
														BOOST_FOREACH(ptree::value_type &userData, userInfo.second)
														{
															if (firstUser)
															{
																outputArray.append("{\"username\":\"");
																firstUser = false;
															}
															else
																outputArray.append(",{\"username\":\"");
															username = userData.second.get<string>("username");
															accesslevel = userData.second.get<string>("accessLevel");
															outputArray.append(username);
															outputArray.append("\",\"accesslevel\":\"");
															outputArray.append(accesslevel);
															outputArray.append("\"}");
														}
													}
													outputArray.append("]");
												}
												if (outputArray != "" && outputArray != "[]")
													arrayData["users"] = outputArray;
												FILE_LOG(logDEBUG) << "WSCallbackBASE64 all_users_info success";
												//send response
												BuildCommandResponse(psdata->outputbuffer, id, command, "success", "", data, arrayData);

											}
											else
											{
												FILE_LOG(logDEBUG) << "WSCallbackBASE64 all_users_info failed";
												BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "List users failed", data, data);
											}
										}
										else if (command == "useradd" && psdata->authorizationLevel >= AuthorizationLevels::SystemAdministrator)
										{
											std::map<string, string> data;
											if (argsList.find("username") != argsList.end() && argsList.find("password") != argsList.end() && argsList.find("accesslevel") != argsList.end())
											{
												bool validPassword = true;
												string passwordError = "User add failed:";
												_tmxControl.ClearOptions();
												_tmxControl.SetOption("username", argsList["username"]);
												_tmxControl.SetOption("password", argsList["password"]);
												_tmxControl.SetOption("access-level", argsList["accesslevel"]);
												//check for 8 or more valid characters in sequence
												if (!boost::regex_match(argsList["password"], boost::regex("[-!@#$%^&*+=<>?0-9a-zA-Z]{8,}")))
												{
													if (argsList["password"].length() < 8)
														passwordError.append(" (Invalid Length)");
													if (argsList["password"].length() < 8)
														passwordError.append(" (Invalid Character)");
													validPassword = false;
												}
												//check that we have at least one character of each type
												if (!boost::regex_search(argsList["password"], boost::regex("[-!@#$%^&*+=<>?]")))
												{
													passwordError.append(" (No Symbol)");
													validPassword = false;
												}
												if (!boost::regex_search(argsList["password"], boost::regex("[0-9]")))
												{
													passwordError.append(" (No Number)");
													validPassword = false;
												}
												if (!boost::regex_search(argsList["password"], boost::regex("[a-z]")))
												{
													passwordError.append(" (No Lower Case Letter)");
													validPassword = false;
												}
												if (!boost::regex_search(argsList["password"], boost::regex("[A-Z]")))
												{
													passwordError.append(" (No Upper Case Letter)");
													validPassword = false;
												}

												if (validPassword)
												{
													bool rc = _tmxControl.user_add();
													if (rc)
													{
														FILE_LOG(logDEBUG) << "WSCallbackBASE64 useradd success";
														BuildCommandResponse(psdata->outputbuffer, id, command, "success", "", data, data);

													}
													else
													{
														FILE_LOG(logDEBUG) << "WSCallbackBASE64 useradd failed";
														BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "User add failed", data, data);
													}
												}
												else
												{
													FILE_LOG(logDEBUG) << "WSCallbackBASE64 useradd failed:" << passwordError;
													BuildCommandResponse(psdata->outputbuffer, id, command, "failed", passwordError, data, data);
												}
											}
											else
											{
												FILE_LOG(logDEBUG) << "WSCallbackBASE64 useradd failed";
												BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "User add failed, missing command arguments", data, data);
											}
										}
										else if (command == "userupdate" && psdata->authorizationLevel >= AuthorizationLevels::SystemAdministrator)
										{
											std::map<string, string> data;
											if (argsList.find("username") != argsList.end() && (argsList.find("password") != argsList.end() || argsList.find("accesslevel") != argsList.end()))
											{
												bool validPassword = true;
												string passwordError = "User update failed:";
												_tmxControl.ClearOptions();
												_tmxControl.SetOption("username", argsList["username"]);
												if (argsList.find("password") != argsList.end())
												{
													//FILE_LOG(logDEBUG) << "WSCallbackBASE64 new password = " << argsList["password"];
													_tmxControl.SetOption("password", argsList["password"]);
													//check for 8 or more valid characters in sequence
													if (!boost::regex_match(argsList["password"], boost::regex("[-!@#$%^&*+=<>?0-9a-zA-Z]{8,}")))
													{
														if (argsList["password"].length() < 8)
															passwordError.append(" (Invalid Length)");
														if (argsList["password"].length() < 8)
															passwordError.append(" (Invalid Character)");
														validPassword = false;
													}
													//check that we have at least one character of each type
													if (!boost::regex_search(argsList["password"], boost::regex("[-!@#$%^&*+=<>?]")))
													{
														passwordError.append(" (No Symbol)");
														validPassword = false;
													}
													if (!boost::regex_search(argsList["password"], boost::regex("[0-9]")))
													{
														passwordError.append(" (No Number)");
														validPassword = false;
													}
													if (!boost::regex_search(argsList["password"], boost::regex("[a-z]")))
													{
														passwordError.append(" (No Lower Case Letter)");
														validPassword = false;
													}
													if (!boost::regex_search(argsList["password"], boost::regex("[A-Z]")))
													{
														passwordError.append(" (No Upper Case Letter)");
														validPassword = false;
													}

												}
												if (validPassword)
												{
													if (argsList.find("accesslevel") != argsList.end())
														_tmxControl.SetOption("access-level", argsList["accesslevel"]);
													bool rc = _tmxControl.user_update();
													if (rc)
													{
														FILE_LOG(logDEBUG) << "WSCallbackBASE64 userupdate success";
														BuildCommandResponse(psdata->outputbuffer, id, command, "success", "", data, data);

													}
													else
													{
														FILE_LOG(logDEBUG) << "WSCallbackBASE64 userupdate failed";
														BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "User update failed", data, data);
													}
												}
												else
												{
													FILE_LOG(logDEBUG) << "WSCallbackBASE64 userupdate failed:" << passwordError;
													BuildCommandResponse(psdata->outputbuffer, id, command, "failed", passwordError, data, data);
												}

											}
											else
											{
												FILE_LOG(logDEBUG) << "WSCallbackBASE64 userupdate failed";
												BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "User update failed, missing command arguments", data, data);
											}
										}
										else if (command == "userdelete" && psdata->authorizationLevel >= AuthorizationLevels::SystemAdministrator)
										{
											std::map<string, string> data;
											if (argsList.find("username") != argsList.end())
											{
												_tmxControl.ClearOptions();
												_tmxControl.SetOption("username", argsList["username"]);
												bool rc = _tmxControl.user_delete();
												if (rc)
												{
													FILE_LOG(logDEBUG) << "WSCallbackBASE64 userdelete success";
													BuildCommandResponse(psdata->outputbuffer, id, command, "success", "", data, data);

												}
												else
												{
													FILE_LOG(logDEBUG) << "WSCallbackBASE64 userdelete failed";
													BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "User delete failed", data, data);
												}
											}
											else
											{
												FILE_LOG(logDEBUG) << "WSCallbackBASE64 userdelete failed";
												BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "User delete failed, missing command arguments", data, data);
											}
										}
										else if (command == "uploadfile" && psdata->authorizationLevel >= AuthorizationLevels::SystemAdministrator)
										{
											std::map<string, string> data;
											std::map<string, string> arrayData;
											data["state"] = "request";
											if (argsList.find("uploadfilename") != argsList.end() && argsList.find("destinationfilename") != argsList.end() && argsList.find("destinationpath") != argsList.end() && argsList.find("filesize") != argsList.end())
											{
												if (boost::regex_match(argsList["filesize"], boost::regex("[0-9]+")))
												{
													//delete all old requests for this connection if they are not uploading
													for (auto it = _uploadRequests.begin();it != _uploadRequests.end();)
													{
														if (it->second.outputbuffer == psdata->outputbuffer && !it->second.uploading)
															it = _uploadRequests.erase(it);
														else
															it++;
													}
													CommandPlugin::UploadData uploadData;
													uploadData.requestId = id;
													uploadData.requestTimeMS = GetMsTimeSinceEpoch();
													uploadData.outputbuffer = psdata->outputbuffer;
													uploadData.destinationFileName = argsList["destinationfilename"];
													uploadData.destinationPath = argsList["destinationpath"];
													uploadData.fileSize = stol(argsList["filesize"]);
													uploadData.uploading = false;
													message = "";
													_uploadRequests[argsList["uploadfilename"]] = uploadData;
													BuildCommandResponse(psdata->outputbuffer, id, command, "success", "", data, arrayData);
												}
												else
												{
													FILE_LOG(logDEBUG) << "WSCallbackBASE64 uploadfile failed";
													BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "Upload file failed, invalid filesize argument", data, arrayData);
												}
											}
											else
											{
												FILE_LOG(logDEBUG) << "WSCallbackBASE64 uploadfile failed";
												BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "Upload file failed, missing command arguments", data, arrayData);
											}
										}
										else if (command == "plugininstall" && psdata->authorizationLevel >= AuthorizationLevels::ApplicationAdministrator)
										{
											std::map<string, string> data;
											if (argsList.find("pluginfile") != argsList.end())
											{
												_tmxControl.ClearOptions();
												string installFile;
												{
													lock_guard<mutex> lock(_configLock);
													installFile = _downloadPath;
												}
												installFile.append("/");
												installFile.append(argsList["pluginfile"]);
												_tmxControl.SetOption("plugin-install", installFile);
												_tmxControl.SetOption("plugin-directory", DEFAULT_PLUGINDIRECTORY);
												bool rc = _tmxControl.plugin_install();
												if (rc)
												{
													FILE_LOG(logDEBUG) << "WSCallbackBASE64 plugininstall success";
													BuildCommandResponse(psdata->outputbuffer, id, command, "success", "", data, data);

												}
												else
												{
													FILE_LOG(logDEBUG) << "WSCallbackBASE64 plugininstall failed";
													BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "Plugin install failed", data, data);
												}
											}
										}
										else if (command == "pluginuninstall" && psdata->authorizationLevel >= AuthorizationLevels::ApplicationAdministrator)
										{
											std::map<string, string> data;
											if (argsList.find("plugin") != argsList.end())
											{
												_tmxControl.ClearOptions();
												_tmxControl.SetOption("plugin-remove", argsList["plugin"]);
												bool rc = _tmxControl.plugin_remove();
												if (rc)
												{
													FILE_LOG(logDEBUG) << "WSCallbackBASE64 pluginuninstall success";
													BuildCommandResponse(psdata->outputbuffer, id, command, "success", "", data, data);

												}
												else
												{
													FILE_LOG(logDEBUG) << "WSCallbackBASE64 pluginuninstall failed";
													BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "Plugin uninstall failed", data, data);
												}
											}
										}
									}
									else
									{
										//only login command can be processed if not authorized
										if (command == "login")
										{
											if (argsList.find("user") != argsList.end() && argsList.find("password") != argsList.end())
											{
												std::map<string, string> data;
												std::map<string, string> arrayData;
												std::map<string, string> passTemp;
												//get user info
												_tmxControl.ClearOptions();
												_tmxControl.SetOption("username", argsList["user"]);
												if (_tmxControl.user_info(true))
												{
													//get json output
													string uJSON = _tmxControl.GetOutput(TmxControlOutputFormat_JSON, false);
													if (uJSON == "")
													{
														//user not found
														BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "Invalid user name", data, arrayData);
													}
													else
													{
														bool authorized = false;
														tmx::message_container_type *output = _tmxControl.GetOutput();
														BOOST_FOREACH(ptree::value_type &userInfo, output->get_storage().get_tree())
														{
															BOOST_FOREACH(ptree::value_type &userData, userInfo.second)
															{
																//check each user record returned
																if (userData.second.get<string>("username") == argsList["user"] && authorized == false)
																{
																	//set access level
																	data["level"] = userData.second.get<string>("accessLevel");
																	passTemp["password"] = userData.second.get<string>("password");
																}
															}
														}
														//verify password
														_tmxControl.ClearOptions();
														_tmxControl.SetOption("password", argsList["password"]);
														if (_tmxControl.hashed_info())
														{
															BOOST_FOREACH(ptree::value_type &userInfo, output->get_storage().get_tree())
															{
																BOOST_FOREACH(ptree::value_type &userData, userInfo.second)
																{
																	if (userData.second.get<string>("password") == passTemp["password"])
																	{
																		//send response
																		authorized = true;
																		BuildCommandResponse(psdata->outputbuffer, id, command, "success", "", data, arrayData);
																		//set session data
																		psdata->authorized = true;
																		psdata->authorizationLevel = stoi(data["level"]);
																	}
																}
															}
														}
														if (!authorized)
														{
															//bad password
															BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "Invalid user name or password", data, arrayData);
														}
													}
												}
												else
												{
													//db query failed
													BuildCommandResponse(psdata->outputbuffer, id, command, "failed", "Cannot get user info", data, arrayData);
												}

											}
										}
									}
								}
							}
							catch (exception & ex)
							{
								//parse error
								FILE_LOG(logDEBUG) << "WSCallbackBASE64 process message exception: ";
							}

						}
					}
				}
				else
				{
					//part of message
					if (psdata->bufferLength >= READ_BUFFER_SIZE)
					{
						//buffer is full, cant process
						//FILE_LOG(logDEBUG) << "WSCallbackBASE64 incoming message is too large";
					}
					else
					{
						//add to buffer
						psdata->buffer[psdata->bufferLength] = msgBytes[i];
						psdata->bufferLength++;
					}
				}
			}
		}
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		{
			//check authorization, minimum level of readonly allows telemetry
			if (psdata->authorized)
			{
				//check if we need to send full telemetry or update
				if (psdata->sendFullTelemetry)
				{
					FILE_LOG(logDEBUG) << "WSCallbackBASE64 Full";
					psdata->sendFullTelemetry = false;
					//set last update time so we dont immediately send update telemetry
					psdata->lastUpdateSendTimeMS = _lastPluginsUpdateTimeMS;
					//build full telemetry
					BuildFullTelemetry(psdata->outputbuffer, "List");
					BuildFullTelemetry(psdata->outputbuffer, "Config");
					BuildFullTelemetry(psdata->outputbuffer, "Status");
					BuildFullTelemetry(psdata->outputbuffer, "State");
					BuildFullTelemetry(psdata->outputbuffer, "Messages");
					BuildFullTelemetry(psdata->outputbuffer, "SystemConfig");
					BuildFullTelemetry(psdata->outputbuffer, "Events");
				}
				else
				{
					//check if time to send updates
					if (psdata->lastUpdateSendTimeMS < _lastPluginsUpdateTimeMS)
					{
						FILE_LOG(logDEBUG) << "WSCallbackBASE64 send updates, " << psdata->session << ", " << psdata->lastUpdateSendTimeMS << ", " << _lastPluginsUpdateTimeMS;
						//set last send time
						psdata->lastUpdateSendTimeMS = _lastPluginsUpdateTimeMS;
						//build updates
						BuildUpdateTelemetry(psdata->outputbuffer, "List");
						BuildUpdateTelemetry(psdata->outputbuffer, "Config");
						BuildUpdateTelemetry(psdata->outputbuffer, "Status");
						BuildUpdateTelemetry(psdata->outputbuffer, "State");
						BuildUpdateTelemetry(psdata->outputbuffer, "Messages");
						BuildUpdateTelemetry(psdata->outputbuffer, "SystemConfig");
						BuildUpdateTelemetry(psdata->outputbuffer, "Events");
						//build removes, only thing we can remove currently is the plugin itself
						BuildRemoveTelemetry(psdata->outputbuffer, "List");
						//BuildRemoveTelemetry(psdata->outputbuffer, "Config");
						//BuildRemoveTelemetry(psdata->outputbuffer, "Status");
						//BuildRemoveTelemetry(psdata->outputbuffer, "State");
						//BuildRemoveTelemetry(psdata->outputbuffer, "Messages");
					}
				}
			}
			uint64_t currentTime = GetMsTimeSinceEpoch();
			//always send heartbeat if connected
			if (currentTime >= psdata->lastHeartbeatSendTimeMS + _heartbeatIntervalMS)
			{
				//set last send time
				psdata->lastHeartbeatSendTimeMS = currentTime;
				BuildFullTelemetry(psdata->outputbuffer, "Heartbeat");
			}

			//check if theres something to send
			if (psdata->outputbuffer->length() > 0)
			{
				//send data
				SendData(psdata->outputbuffer, wsi);
			}
		}
		break;
	default:
		break;
	}

	return 0;
}

int CommandPlugin::Main()
{
	PLOG(logINFO) << "Starting plugin.";

	struct lws_context *context;

	// list of supported protocols and callbacks
	struct lws_protocols protocols[] = {
			{
					"http",
					(lws_callback_function*)&CommandPlugin::WSCallbackHTTP,
					sizeof(struct PerSessionDataHTTP),
					4096,
					0, NULL, 0
			},
			{
					"base64",
					(lws_callback_function*)&CommandPlugin::WSCallbackBASE64,
					sizeof(struct PerSessionDataBASE64),
					0
			},
			{
					NULL, NULL, 0
			}
	};

	//ignore SIGPIPE caused by connection dropped because of rejected certificate
	signal(SIGPIPE, SIG_IGN);

	lws_context_creation_info info;
	memset(&info, 0, sizeof(info));

	info.port = 19760;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
	info.timeout_secs_ah_idle = 3600;

	string crtPath = "";
	string keyPath = "";
	int opts = 0;

	//wait for initial config values
	while (!_newConfigValues)
		this_thread::sleep_for(std::chrono::milliseconds(_sleepMS));

	if (_sslEnabled)
	{
		opts |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
		//opts |= LWS_SERVER_OPTION_REDIRECT_HTTP_TO_HTTPS;
		lock_guard<mutex> lock(_configLock);
		crtPath = _sslPath;
		crtPath.append("/tmxcmd.crt");
		info.ssl_cert_filepath = crtPath.c_str();
		keyPath = _sslPath;
		keyPath.append("/tmxcmd.key");
		info.ssl_private_key_filepath = keyPath.c_str();
		info.ssl_cipher_list = "ECDHE-ECDSA-AES256-GCM-SHA384:"
					       "ECDHE-RSA-AES256-GCM-SHA384:"
					       "DHE-RSA-AES256-GCM-SHA384:"
					       "ECDHE-RSA-AES256-SHA384:"
					       "HIGH:!aNULL:!eNULL:!EXPORT:"
					       "!DES:!MD5:!PSK:!RC4:!HMAC_SHA1:"
					       "!SHA1:!DHE-RSA-AES128-GCM-SHA256:"
					       "!DHE-RSA-AES128-SHA256:"
					       "!AES128-GCM-SHA256:"
					       "!AES128-SHA256:"
					       "!DHE-RSA-AES256-SHA256:"
					       "!AES256-GCM-SHA384:"
					       "!AES256-SHA256";
		PLOG(logDEBUG) << "SSL enabled";

	}

	info.options = opts;

	// create libwebsocket context representing this server
	context = lws_create_context(&info);

	if (context == NULL)
	{
		PLOG(logDEBUG) << "libwebsocket context create failed";
		return -1;
	}

	PLOG(logDEBUG) << "libwebsocket context created";

	while (_plugin->state != IvpPluginState_error)
	{
		if (_newConfigValues)
		{
			EventLogMessage msg;
			uint64_t currentTime = GetMsTimeSinceEpoch();
			//check if plugins data needs updated only if we have connections
			if (_connectionCount > 0 && currentTime >= _lastPluginsUpdateTimeMS + _updateIntervalMS)
			{
				//set update time
				_lastPluginsUpdateTimeMS = currentTime;
				//update data
				GetTelemetry("List");
				GetTelemetry("Config");
				GetTelemetry("Status");
				GetTelemetry("State");
				GetTelemetry("Messages");
				GetTelemetry("SystemConfig");
				GetEventTelemetry();
			}

			//schedule writable callback for all base64 protocol connections
			lws_callback_on_writable_all_protocol(context, &protocols[1]);

			//service connections
			int sleepTime = _sleepMS;
			lws_service(context, sleepTime);

		}

		//sleep
		this_thread::sleep_for(std::chrono::milliseconds(_sleepMS));
	}

	lws_context_destroy(context);
	PLOG(logDEBUG) << "libwebsocket context destroyed";


	return (EXIT_SUCCESS);
}


} /* namespace CommandPlugin */

int main(int argc, char *argv[])
{
	return run_plugin<CommandPlugin::CommandPlugin>("CommandPlugin", argc, argv);
}
