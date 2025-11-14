/*
 * TimPlugin.h
 *
 *  Created on: October 25, 2017
 *      Author: zinkg
 */

#ifndef TIMPLUGIN_H_
#define TIMPLUGIN_H_

#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <time.h>
#include <atomic>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>


#include <PluginUtil.h>
#include <PluginClient.h>




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
#include <qserverPedestrian/OAIApiRouter.h>
#include <qserverPedestrian/OAIPSM.h>
#include <tmx/j2735_messages/TravelerInformationMessage.hpp>

#include <filesystem> // Required for std::filesystem
#include <TimUtils.hpp>










namespace TimPlugin {


	class TimPlugin: public tmx::utils::PluginClient {
		public:
			explicit TimPlugin(const std::string &name);
			int Main() override ;

		protected:

			void UpdateConfigSettings();

			// Virtual method overrides.
			void OnConfigChanged(const char *key, const char *value) override;
			void OnStateChange(IvpPluginState state) override;

			/**
			 * @brief Calculate tim duration based on the J2735 TIM message startTime and Duration
			 */
			bool TimDuration(std::shared_ptr<tmx::messages::TimMessage> TimMsg);
			/**
			 * @brief Read map file and populate TIM message
			 * @param TimMsg A shared pointer to the TIM object to be updated
			 * @param mapFile File path that has the standard J2735 TIM message in XML format
			*/
			bool LoadTim(std::shared_ptr<tmx::messages::TimMessage> TimMsg, const char *mapFile);
			int  StartWebService();
			void TimRequestHandler(QHttpEngine::Socket *socket);
			void writeResponse(int responseCode , QHttpEngine::Socket *socket);


		private:

			pthread_mutex_t _settingsMutex = PTHREAD_MUTEX_INITIALIZER;
			pthread_mutex_t _timMutex = PTHREAD_MUTEX_INITIALIZER;

			uint64_t _frequency = 0;

			std::string _timupdate; 
			uint16_t webport;
			std::string webip; 

			std::shared_ptr<tmx::messages::TimMessage> _timMsgPtr;

			mutex _mapFileLock;
			string _mapFile;
			std::ofstream tmpTIM;
			std::atomic<bool> _isTimFileNew{false};
			//Post request to update TIM
			std::atomic<bool> _isTimUpdated{false};
			bool _isTimLoaded = false;
			unsigned int _speedLimit = 0;
			int _lastMsgIdSent = -1;
			std::mutex _cfgLock;


	};
}
#endif
