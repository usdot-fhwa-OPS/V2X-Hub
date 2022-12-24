
#ifndef ERVCLOUDFORWARDINGWORKER_H_
#define ERVCLOUDFORWARDINGWORKER_H_

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <tmx/tmx.h>
#include <tmx/messages/IvpJ2735.h>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>

using namespace std;
using namespace tmx;
using namespace tmx::messages;

namespace ERVCloudForwardingPlugin
{
    class ERVCloudForwardingWorker
    {
    public:
        ERVCloudForwardingWorker() = delete;
        ~ERVCloudForwardingWorker() = delete;
        /**
         * @brief Encode BSM object and return an encoded hex string
         * 
         * @param msg The BSM object to encode
         * @return Hex string 
         */
        static string encodeBSMHex(BsmMessage &msg);
        /**
         * @brief Create a BSM request if the BSM is sent from an ERV(Emergency Response Vehicle)
         * @param msg The BSM object
         * @return The BSM request in XML format
         */
        static string constructERVBSMRequest(BsmMessage &msg);
        /**
         * @brief Check whether the BSM is sent from an ERV
         * @param msg The BSM object
         * @return true if the BSM is from ERV
         * @return false if the BSM is NOT from ERV
         */
        static bool IsBSMFromERV(BsmMessage &msg);
    };
}
#endif