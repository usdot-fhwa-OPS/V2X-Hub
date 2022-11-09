/*
 * TimPlugin.h
 *
 *  Created on: October 25, 2017
 *      Author: zinkg
 */

#ifndef CARMACLOUDPLUGIN_H_
#define CARMACLOUDPLUGIN_H_

#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <time.h>
#include <sys/time.h>
#include <atomic>
#include <tmx/tmx.h>
#include <tmx/IvpPlugin.h>
#include <tmx/messages/IvpJ2735.h>
#include <GeoVector.h>
#include <boost/date_time.hpp>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <zlib.h>


#include "PluginUtil.h"
#include "PluginClient.h"

#include <ApplicationMessage.h>
#include <ApplicationDataMessage.h>

#include <tmx/messages/auto_message.hpp>
#include <tmx/messages/IvpJ2735.h>
#include <tmx/j2735_messages/testMessage04.hpp>
#include <TestMessage04.h>
#include <tmx/j2735_messages/testMessage05.hpp>
#include <TestMessage05.h>
#include <tmx/messages/auto_message.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>


#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QHostAddress>
#include <QRegExp>
#include <QStringList>
#include <QSharedPointer>
#include <QObject>

#ifdef __linux__
#include <signal.h>
#include <unistd.h>
#endif
#include <qhttpengine/server.h>
#include <v2xhubWebAPI/OAIApiRouter.h>


#include <curl/curl.h>
#include <algorithm>



using namespace std;

using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace OpenAPI;


namespace CARMACloudPlugin {
	
enum acknowledgement_status {
	acknowledgement_status__acknowledged 		= 1, //Acknowledged by CMV
	acknowledgement_status__rejected 			= 2, //CMV cannot process the TCM message
	acknowledgement_status__not_acknowledged 	= 3  //CMV does not respond at all within the v2xhub repeatedly broadcast time period
};

class CARMACloudPlugin: public PluginClient {
public:
	CARMACloudPlugin(std::string);
	virtual ~CARMACloudPlugin();
	int Main();
	uint16_t webport;
	std::string webip;
	uint16_t fetchtime;
protected:

	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);
	//void OnMessageReceived(IvpMessage *msg);
	void OnStateChange(IvpPluginState state);

	int  StartWebService();
	void CARMAResponseHandler(QHttpEngine::Socket *socket);
	int CloudSend(const string& msg,const string& url, const string& base, const string& method);
	//Send HTTP request async
	void CloudSendAsync(const string& msg,const string& url, const string& base, const string& method);
	string updateTags(string s,string t, string t1);

	void HandleCARMARequest(tsm4Message &msg, routeable_message &routeableMsg);
	void HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg);
	void GetInt32(unsigned char *buf, int32_t *value)
	{
		*value = (int32_t)((buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3]);
	}
	
	void GetInt64(unsigned char *buf, uint64_t *value)
	{
		*value = (uint64_t)((buf[0] << 56) + (buf[1] << 48) + (buf[2] << 40) + (buf[3] << 32) + (buf[4] << 24) + (buf[5] << 16) + (buf[6] << 8) + buf[7]);
	}

	/***
	 * @brief: Insert the comma separated string into the  list
	 * @param: std::vector of substrings
	 * @param: comma separated string 
	 * **/
	void ConvertString2Vector(std::vector<string> &sub_str_v, const string &str) const;

	/***
	 * @brief: Insert the colon separated string into the pair
	 * @param: std::pair<string, string> of string
	 * @param: colon separated string 
	 * **/
	void ConvertString2Pair(std::pair<string,string> &str_pair, const string &str) const;

	/***
	 * @brief: Retrieve the value string from the strategy params based on the input key string
	 * @param: std::vector of comma separated string in stragety_params
	 * @param: key string to identify the value from the key-value pair separated by colon. Input key should be all lower case
	 * **/
	string GetValueFromStrategyParamsByKey(const std::vector<string> & stragety_params_v, const string &key) const;
	/***
	 * @brief: Loop through the received TCMs and broadcast them for the configured duration.
	 * If it timed out, it would remove the TCMs from the list, and stop broadcasting them.
	 * ***/
	void TCMAckCheckAndRebroadcastTCM();
	/***
	 * @biref: Add DSRC metadata for TCM and broadcast TCM
	 * @param: Encoded TCM to broadcast
	 ***/
	void BroadcastTCM(tsm5EncodedMessage& tsm5ENC);
	/***
	 * @brief: Determin if stop broadcasting the current TCM
	 * @param: std::string of decoded TCM request id
	 * @param: key string TCM hex payload
	 * **/
	bool IsSkipBroadcastCurTCM(const string & tcmv01_req_id_hex, const string & tcm_hex_payload ) const;

	/**
	 * @brief Uncompress input source buffer
	 * @param compressedBytes Uncompressed source buffer
	 * @return QByteArray uncompressed destination buffer
	 */
	QByteArray UncompressBytes(const QByteArray compressedBytes) const;
	/***
	 * @brief Filter each individual TCM from the tcm response. 
	 * @param tcm_response string in XML format. The TCM response string can be either a list of TCMs or one TCM.
	 * @return A list of TCM in XML format
	*/
	std::list<std::string> FilterTCMs(const std::string& tcm_response) const;

private:

	pthread_mutex_t _settingsMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t _timMutex = PTHREAD_MUTEX_INITIALIZER;
	J2735MessageFactory factory;
	uint64_t _frequency = 0;
	string url;
	string base_hb;
	string base_req;
	string method;
	string base_ack; 
	
	//Comma separated string for list of strategies from MobilityOperation messages
	std::string _strategies;

	struct TCMBroadcastMetadata
	{
		string tcm_hex;
		int num_of_times;
	};
	//Used to lock the shared TCMs resource
	std::mutex _not_ACK_TCMs_mutex;
	//An associated array to keep track of TCMs that are not acknowledged
	std::shared_ptr<std::multimap<string, tsm5EncodedMessage>> _not_ACK_TCMs;
	
	//TCM repeatedly broadcast time out in unit of second
	uint16_t _TCMRepeatedlyBroadcastTimeOut = 0;
	//Keep track of the starting time (unit of milliseconds) TCMs with same TCR request ids being broadcast Key: tcm request id, value: the start broadcasting timestamp
	std::shared_ptr<std::map<string, std::time_t>> _tcm_broadcast_starting_time;
	
	std::string _TCMNOAcknowledgementDescription = "";

	//Total number of times repeatedly broadcast TCMs with the same request id
	int _TCMRepeatedlyBroadCastTotalTimes = 0; 
	//Keep track of the number of times repeatedly broadcast TCMS. Key: tcm hex string, value: the number of times
	std::shared_ptr< std::multimap<string, TCMBroadcastMetadata>> _tcm_broadcast_times;

	const string _TCMAcknowledgementStrategy = "carma3/geofence_acknowledgement";
	int _TCMRepeatedlyBroadcastSleep = 100;
	const char *CONTENT_ENCODING_KEY = "Content-Encoding";
    const char *CONTENT_ENCODING_VALUE = "gzip";
	std::string list_tcm = "true";
	
};
std::mutex _cfgLock;
}
#endif
