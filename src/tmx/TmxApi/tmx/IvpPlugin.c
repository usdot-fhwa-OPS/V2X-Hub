/*
 * IvpPluginApi.c
 *
 *  Created on: Jul 19, 2014
 *      Author: ivp
 */

#include "IvpPlugin.h"
#include "utils/MsgFramer.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

const IvpPluginInformation IVP_PLUGIN_INFORMATION_INITIALIZER = { .onMsgReceived = NULL, .onStateChange = NULL, .onError = NULL, .onConfigChanged = NULL, .manifestLocation = NULL };

void ivp_broadcastAndDestroyMessage(IvpPlugin *plugin, IvpMessage *msg);
char *ivp_getManifestFile(const char *filepath);
void ivp_onError(IvpPlugin *plugin, IvpError err);
void ivp_onErrorFromInfo(IvpPluginInformation *info, IvpError err);
void ivp_onStateChange(IvpPlugin *plugin, IvpPluginState state);
void ivp_onMessageReceived(IvpPlugin *plugin, IvpMessage *msg);
void ivp_onConfigChanged(IvpPlugin *plugin, const char *value, const char *key);
void *ivp_receive(void *arg);

IvpPlugin *ivp_create(IvpPluginInformation info)
{
	char *rawManifest = ivp_getManifestFile(info.manifestLocation == NULL ? IVPREGISTER_MANIFEST_FILE_NAME : info.manifestLocation);
	if (rawManifest == NULL)
	{
		ivp_onErrorFromInfo(&info, ivpError_createError(IvpLogLevel_fatal, IvpError_manifestLoad, errno));
		return NULL;
	}

	cJSON *jsonManifest = cJSON_Parse(rawManifest);
	free(rawManifest);
	if (jsonManifest == NULL)
	{
		ivp_onErrorFromInfo(&info, ivpError_createError(IvpLogLevel_fatal, IvpError_manifestParse, errno));
		return NULL;
	}

	IvpPlugin *results = calloc(1, sizeof(IvpPlugin));
	if (results == NULL)
	{
		cJSON_Delete(jsonManifest);
		return NULL;
	}

	results->info = info;
	results->jsonManifest = jsonManifest;

	pthread_mutexattr_t lockAttr;
	pthread_mutexattr_init(&lockAttr);
	pthread_mutexattr_settype(&lockAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&results->lock, &lockAttr);

	IvpManifest *manifest = ivpRegister_getManifestFromJson(results->jsonManifest);
	if (manifest != NULL)
	{
		IvpError manifestError = ivpRegister_validateManifest(manifest);
		if (manifestError.error == IvpError_none)
		{
			results->coreIpAddr = manifest->coreIpAddr ? strdup(manifest->coreIpAddr) : strdup(IVP_DEFAULT_IP);
			results->corePortNumber = manifest->corePort ? manifest->corePort : IVP_DEFAULT_PORT;
			if (manifest->configuration != NULL) results->config = cJSON_Duplicate(manifest->configuration, 1);
		}

		ivpRegister_destroyManifest(manifest);

		if (manifestError.error == IvpError_none)
		{
			pthread_create(&results->receiveThread, NULL, ivp_receive, results);
			//TODO: Spawn monitor thread
			return results;
		}
		else
		{
			ivp_onError(results, manifestError);
		}
	}
	else
	{
		ivp_onError(results, ivpError_createError(IvpLogLevel_fatal, IvpError_manifestExtract, 0));
	}

	ivp_destroy(results);
	return NULL;
}

void ivp_subscribe(IvpPlugin *plugin, IvpMsgFilter *filter)
{
	assert(plugin != NULL);
	if (plugin == NULL)
		return;

	pthread_mutex_lock(&plugin->lock);

	if (plugin->filter != filter)
	{
		if (plugin->filter != NULL)
			cJSON_Delete(plugin->filter);
		plugin->filter = NULL;
		if (filter != NULL)
			plugin->filter = cJSON_Duplicate(filter, 1);
	}

	ivp_broadcastAndDestroyMessage(plugin, ivpSubscribe_createMsg(plugin->filter));

	pthread_mutex_unlock(&plugin->lock);
}

void ivp_setStatus(IvpPlugin *plugin, const char *status)
{
	assert(plugin != NULL);
	assert(status != NULL);
	if (plugin == NULL || status == NULL)
		return;

	IvpPluginStatusCollection *collection = NULL;
	collection = ivpPluginStatus_addStatusItem(collection, NULL, status);

	ivp_broadcastAndDestroyMessage(plugin, ivpPluginStatus_createMsg(collection));

	if (collection != NULL)
		ivpPluginStatus_destroyCollection(collection);
}

void ivp_setStatusItem(IvpPlugin *plugin, const char *key, const char *value)
{
	assert(plugin != NULL);
	assert(key != NULL);
	assert(key[0] != '\0');
	assert(value != NULL);
	if (plugin == NULL || key == NULL || key[0] == '\0' || value == NULL)
		return;

	IvpPluginStatusCollection *collection = NULL;
	collection = ivpPluginStatus_addStatusItem(collection, key, value);

	ivp_broadcastAndDestroyMessage(plugin, ivpPluginStatus_createMsg(collection));

	if (collection != NULL)
		ivpPluginStatus_destroyCollection(collection);
}

void ivp_removeStatusItem(IvpPlugin *plugin, const char *key)
{
	assert(plugin != NULL);
	assert(key != NULL);
	assert(key[0] != '\0');
	if (plugin == NULL || key == NULL || key[0] == '\0')
		return;

	IvpPluginStatusCollection *collection = NULL;
	collection = ivpPluginStatus_addStatusItem(collection, key, NULL);

	ivp_broadcastAndDestroyMessage(plugin, ivpPluginStatus_createMsg(collection));

	if (collection != NULL)
		ivpPluginStatus_destroyCollection(collection);
}

void ivp_addEventLog(IvpPlugin *plugin, IvpLogLevel level, const char *description)
{
	assert(plugin != NULL);
	assert(description != NULL);
	assert(description[0] != '\0');
	if (plugin == NULL || description == NULL || description[0] == '\0')
		return;

	ivp_broadcastAndDestroyMessage(plugin, ivpEventLog_createMsg(level, description));
}

void ivp_broadcastMessage(IvpPlugin *plugin, IvpMessage *msg)
{
	assert(plugin != NULL);
	assert(msg != NULL);
	if (plugin == NULL || msg == NULL)
		return;

	if (plugin->state != IvpPluginState_connected && plugin->state != IvpPluginState_registered)
		return;

	char *jsonMsg = ivpMsg_createJsonString(msg, IvpMsg_FormatOptions_none);
	if (jsonMsg != NULL)
	{
		int framedLength;
		char *framedMsg = msgFramer_createFramedMsg(jsonMsg, strlen(jsonMsg), &framedLength);
		if (framedMsg != NULL)
		{
			pthread_mutex_lock(&plugin->lock);
			if (plugin->state == IvpPluginState_connected || plugin->state == IvpPluginState_registered)
			{
				if (send(plugin->socket, framedMsg, framedLength, 0) <= 0)
					ivp_onStateChange(plugin, IvpPluginState_disconnected);
			}
			pthread_mutex_unlock(&plugin->lock);

			free(framedMsg);
		}
		free(jsonMsg);
	}
}

char *ivp_getCopyOfConfigurationValue(IvpPlugin *plugin, const char *key)
{
	assert(plugin != NULL);
	assert(key != NULL);
	assert(key[0] != '\0');
	if (plugin == NULL || key == NULL || key[0] == '\0')
		return NULL;

	if (plugin->config == NULL)
	{
		ivp_onError(plugin, ivpError_createError(IvpLogLevel_error, IvpError_configKeyDoesntExist, 0));
		return NULL;
	}

	pthread_mutex_lock(&plugin->lock);
	char *results = ivpConfig_getCopyOfValueFromCollection(plugin->config, key);
	pthread_mutex_unlock(&plugin->lock);

// Commenting out the two lines below to prevent errors being logged to the event log when a configuration
// parameter does not exist.  MGB 1/31/2017
//	if (results == NULL)
//		ivp_onError(plugin, ivpError_createError(IvpLogLevel_error, IvpError_configKeyDoesntExist, 0));

	return results;
}

void ivp_setConfigurationValue(IvpPlugin *plugin, const char *key, const char *value)
{
	assert(plugin != NULL);
	assert(key != NULL);
	assert(key[0] != '\0');
	assert(value != NULL);
	if (plugin == NULL || key == NULL || key[0] == '\0' || value == NULL)
		return;

	//TODO HACK this doesn't save anything locally? should we.  or return true/false on success/error.  Issue is that if we aren't registered this fails...
	IvpConfigCollection *collection = NULL;
	collection = ivpConfig_addItemToCollection(collection, key, value, NULL);

	ivp_broadcastAndDestroyMessage(plugin, ivpConfig_createMsg(collection));

	if (collection != NULL)
		ivpConfig_destroyCollection(collection);
}

void ivp_destroy(IvpPlugin *plugin)
{
	assert(plugin != NULL);
	if (plugin == NULL)
		return;

	pthread_cancel(plugin->receiveThread);
	pthread_join(plugin->receiveThread, NULL);

	if (plugin->jsonManifest != NULL)
		cJSON_Delete(plugin->jsonManifest);
	if (plugin->filter != NULL)
		cJSON_Delete(plugin->filter);

	if (plugin->coreIpAddr != NULL)
		free(plugin->coreIpAddr);

	free(plugin);
}

void ivp_broadcastAndDestroyMessage(IvpPlugin *plugin, IvpMessage *msg)
{
	assert(plugin != NULL);
	if (plugin == NULL)
		return;

	if (msg != NULL)
	{
		ivp_broadcastMessage(plugin, msg);
		ivpMsg_destroy(msg);
	}
}

char *ivp_getManifestFile(const char *filepath)
{
	assert(filepath != NULL);
	if (filepath == NULL)
		return NULL;

	errno = 0;

	FILE *fd = fopen(filepath, "r");
	if (fd != NULL)
	{
		int length = 0;
		if (fseek(fd, 0, SEEK_END) >= 0
				&& (length = ftell(fd)) > 0
				&& fseek(fd, 0, SEEK_SET) >= 0)
		{
			char *contents = malloc(length + 1);
			{
				if (contents != NULL)
				{
					if (fread(contents, 1, length, fd) == length)
					{
						contents[length] = '\0';
						return contents;
					}

					free(contents);
				}
			}
		}
	}

	return NULL;
}


void ivp_onError(IvpPlugin *plugin, IvpError err)
{
	assert(plugin != NULL);
	if (plugin == NULL)
		return;

	if (err.error == IvpError_none)
		return;

	if (err.level >= IvpLogLevel_fatal)
		plugin->state = IvpPluginState_error;

	if (plugin->info.onError != NULL)
		plugin->info.onError(plugin, err);
}

void ivp_onErrorFromInfo(IvpPluginInformation *info, IvpError err)
{
	assert(info != NULL);
	if (info == NULL)
		return;

	if (err.error == IvpError_none)
		return;

	if (info->onError != NULL)
		info->onError(NULL, err);
}

void ivp_onStateChange(IvpPlugin *plugin, IvpPluginState state)
{
	assert(plugin != NULL);
	if (plugin == NULL)
		return;

	if (plugin->state == state)
		return;

	plugin->state = state;
	if (plugin->info.onStateChange != NULL)
		plugin->info.onStateChange(plugin, state);
}

void ivp_onMessageReceived(IvpPlugin *plugin, IvpMessage *msg)
{
	assert(plugin != NULL);
	assert(msg != NULL);
	if (plugin == NULL || msg == NULL)
		return;

	if (ivpError_isErrMsg(msg))
	{
		IvpError err = ivpError_getError(msg);
		ivp_onError(plugin, err);
	}
	else if(ivpConfig_isConfigMsg(msg))
	{
		IvpConfigCollection *collection = msg->payload;

		pthread_mutex_lock(&plugin->lock);

		int arraySize = ivpConfig_getItemCount(collection);
		int i;
		for(i = 0; i < arraySize; i++)
		{
			IvpConfigItem *item = ivpConfig_getItem(collection, i);
			assert(item != NULL);
			if (item != NULL)
			{
				assert(item->key != NULL);
				if (item->key && item->value)
				{
					if (ivpConfig_updateValueInCollection(plugin->config, item->key, item->value))
						ivp_onConfigChanged(plugin, item->key, item->value);
				}

				ivpConfig_destroyConfigItem(item);
			}
		}

		pthread_mutex_unlock(&plugin->lock);

		if (plugin->state == IvpPluginState_connected)
		{
			ivp_onStateChange(plugin, IvpPluginState_registered);
			ivp_subscribe(plugin, plugin->filter);
		}
	}
	else
	{
		if (plugin->info.onMsgReceived != NULL)
			plugin->info.onMsgReceived(plugin, msg);
	}
}

void ivp_onConfigChanged(IvpPlugin *plugin, const char *key, const char *value)
{
	assert(plugin != NULL);
	assert(key != NULL);
	assert(value != NULL);
	if (plugin == NULL || key == NULL || value == NULL)
		return;

	if (plugin->info.onConfigChanged != NULL)
		plugin->info.onConfigChanged(plugin, key, value);
}

void *ivp_receive(void *arg)
{
	IvpPlugin *plugin = (IvpPlugin *)arg;
	assert(plugin != NULL);
	if (plugin == NULL)
		return NULL;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(plugin->coreIpAddr);
	addr.sin_port = htons(plugin->corePortNumber);

	while(plugin->state != IvpPluginState_error)
	{
		pthread_testcancel();

		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd == -1)
		{
			sleep(1);
			ivp_onError(plugin, ivpError_createError(IvpLogLevel_error, IvpError_connectFail, errno));
			continue;
		}
		if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		{
			ivp_onError(plugin, ivpError_createError(IvpLogLevel_error, IvpError_connectFail, errno));
			close(fd);
			sleep(2);
			continue;
		}

		plugin->socket = fd;
		ivp_onStateChange(plugin, IvpPluginState_connected);

		ivp_broadcastAndDestroyMessage(plugin, ivpRegister_createMsgFromJson(plugin->jsonManifest));

		MsgFramer framer = MSG_FRAMER_INITIALIZER;

		while(plugin->state == IvpPluginState_connected
				|| plugin->state == IvpPluginState_registered)
		{
			int recvcount = recv(fd, msgFramer_getBuf(&framer), msgFramer_getBufLength(&framer), 0);
			if (recvcount <= 0)
			{
				if (recvcount < 0)
				{
					ivp_onError(plugin, ivpError_createError(IvpLogLevel_error, IvpError_connectionDropped, errno));
				}
				ivp_onStateChange(plugin, IvpPluginState_disconnected);
			}
			else
			{
				msgFramer_incrementBufPos(&framer, recvcount);

				char *rawmsg = NULL;

				while((rawmsg = msgFramer_getNextMsg(&framer)) != NULL)
				{
					IvpMessage *msg = ivpMsg_parse(rawmsg);
					if (msg != NULL)
					{
						ivp_onMessageReceived(plugin, msg);
						ivpMsg_destroy(msg);
					}
					else
					{
						ivp_onError(plugin, ivpError_createError(IvpLogLevel_warn, IvpError_messageParse, 0));
					}
				}
			}
		}

		close(fd);
		plugin->socket = -1;
	}

	return NULL;
}

