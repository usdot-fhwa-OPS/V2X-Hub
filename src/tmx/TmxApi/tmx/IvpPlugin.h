/*
 * IvpPlugin.h
 *
 *  Created on: Jul 19, 2014
 *      Author: ivp
 */

#ifndef IVPPLUGIN_H_
#define IVPPLUGIN_H_

#include "IvpMessage.h"
#include "apimessages/IvpError.h"
#include "apimessages/IvpSubscribe.h"
#include "apimessages/IvpConfig.h"
#include "apimessages/IvpPluginStatus.h"
#include "apimessages/IvpRegister.h"
#include "apimessages/IvpEventLog.h"
#include "apimessages/IvpMessageType.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * Different status states of a plugin
 */
typedef enum {
	IvpPluginState_disconnected,
	IvpPluginState_connected,
	IvpPluginState_registered,
	IvpPluginState_error
} IvpPluginState;

//Forward declaration
struct IvpPlugin;

/*!
 * A structure to provide the plugin api with the proper information to register the plugin and callbacks to allow
 * the plugin to pass information back to the application.
 */
typedef struct {
	/*!
	 * A callback that the api will use to pass received messages back to the application.  The api will destroy the message after this callback returns.
	 * If the application wants to hold onto the message for longer, it must explicitly make a copy of it.
	 *
	 * NULL is allowed.
	 */
	void (*onMsgReceived)(struct IvpPlugin *plugin, IvpMessage *msg);
	/*!
	 *
	 * NULL is allowed.
	 */
	void (*onStateChange)(struct IvpPlugin *plugin, IvpPluginState state);
	/*!
	 * If a fatal error is ever returns, the plugin will go into an error state and the connection will be severed.
	 *
	 * NULL is allowed.
	 */
	void (*onError)(struct IvpPlugin *plugin, IvpError err);
	/*!
	 *
	 * NULL is allowed.
	 */
	void (*onConfigChanged)(struct IvpPlugin *plugin, const char *key, const char *value);

	char *manifestLocation;

} IvpPluginInformation;

/*!
 * A static initializer to create a safe IvpPluginInformation structure.
 */
extern const IvpPluginInformation IVP_PLUGIN_INFORMATION_INITIALIZER;

typedef struct IvpPlugin {
	IvpPluginInformation info;
	IvpPluginState state;
	cJSON *jsonManifest;
	IvpMsgFilter *filter;
	IvpConfigCollection *config;
	char *coreIpAddr;
	int corePortNumber;
	int socket;
	pthread_t receiveThread;
	pthread_mutex_t lock;
} IvpPlugin;

/*!
 *
 * @returns
 * 		A malloc'd and initialized plugin, otherwise NULL on error.
 */
IvpPlugin *ivp_create(IvpPluginInformation info);

/*!
 *
 * @requires
 * 		plugin != NULL
 */
void ivp_subscribe(IvpPlugin *plugin, IvpMsgFilter *filter);

/*!
 *
 * @requires
 * 		plugin != NULL
 * 		status != NULL
 */
void ivp_setStatus(IvpPlugin *plugin, const char *status);

/*!
 *
 * @requires
 * 		plugin != NULL
 * 		key != NULL
 * 		length(key) > 0
 * 		value != NULL
 */
void ivp_setStatusItem(IvpPlugin *plugin, const char *key, const char *value);

/*!
 *
 * @requires
 * 		plugin != NULL
 * 		key != NULL
 * 		length(key) > 0
 */
void ivp_removeStatusItem(IvpPlugin *plugin, const char *key);

/*!
 *
 * @requires
 * 		plugin != NULL
 * 		description != NULL
 * 		length(description) > 0
 */
void ivp_addEventLog(IvpPlugin *plugin, IvpLogLevel level, const char *description);

/*!
 *
 * @requires
 * 		plugin != NULL
 * 		msg != NULL
 */
void ivp_broadcastMessage(IvpPlugin *plugin, IvpMessage *msg);

/*!
 *
 * @requires
 * 		plugin != NULL
 * 		key != NULL
 * 		length(key) > 0
 */
char *ivp_getCopyOfConfigurationValue(IvpPlugin *plugin, const char *key);

/*!
 *
 * @requires
 * 		plugin != NULL
 * 		key != NULL
 * 		length(key) > 0
 * 		value != NULL
 */
void ivp_setConfigurationValue(IvpPlugin *plugin, const char *key, const char *value);

/*!
 *
 * @requires
 * 		plugin != NULL
 */
void ivp_destroy(IvpPlugin *plugin);


#ifdef __cplusplus
}
#endif

#endif /* IVPPLUGIN_H_ */
