/**
 * Copyright (C) 2025 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
#pragma once
#include <libwebsockets.h>
#include <string>
#include <iostream>
#include <mutex>
#include "PluginLog.h"
#include <atomic>
#include <vector>
#include <thread>

using tmx::utils::logERROR;
using tmx::utils::logDEBUG;
using tmx::utils::logDEBUG1;
using tmx::utils::logDEBUG2;
using tmx::utils::logDEBUG3;
using tmx::utils::logDEBUG4;
using std::vector;
using std::lock_guard;
using std::mutex;
using std::string;


namespace CDA1TenthPlugin{
  class WebSocketServer
  {
  private:
      static const  int PER_SESSION_USER_DATA_SIZE = 0;
      static const int MAX_FRAME_SIZE = 4096;
      static const int PORT = 9002;
      static const int PULL_PERIOD = 10; // milliseconds
      static const int LWS_USEC_PER_MS = 1000; // 1 millisecond = 1000 microseconds

    //Websocket context  
    lws_context *context;
    
    //Websocket context creation info 
    lws_context_creation_info context_info;

    //List of connected clients
    vector<lws*> clients;

    //Mutex for client list
    mutex clients_mutex;

    //Flag to indicate if the server is running
    std::atomic<bool> running{false}; 

    //List of messages to send
    vector<string> message_queue;

    //Mutex for message list
    mutex message_queue_mutex;

  public:
    WebSocketServer();
    ~WebSocketServer();
    /**
     * @brief Websocket callback function for handling websocket events 
     * @param wsi The websocket instance
     * @param reason The reason for the callback
     * @param user User data
     * @param in Incoming data
     * @param len Length of incoming data
     * @return int 
     */
    static int webSocketCallback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
   
   /**
    * @brief Remove a client from the list of connected clients
    * @param wsi The client to remove
    */
    void removeLWSClients(lws* wsi);

   /**
    * @brief Add a client to the list of connected clients
    * @param wsi The client to add
    */
    void addLWSClients(lws* wsi);

    /**
     * @brief Run the websocket server
     */
    void run();

    /**
     * @brief Get the running flag
     * @return bool 
     */
    bool isRunning();

    /**
     * @brief Set the running flag
     * @param value The value to set
     */
    void setRunning(bool value);

    /**
     * @brief Add a message to the list of messages to send
     * @param msg The message to add
     */
    void addMessage(std::string msg);

    /**
    * @brief Send a message to all connected clients
    * @param msg The message to send
    */
    void sendMsgToAllClients(const std::string& msg);

    /**
     * @brief Send all queued messages to all connected clients
     */
    void sendQueuedMessagesToAllClients();

    /**
     * @brief Send message to a client
     */
    void sendMessage(lws* wsi, const string& message) const;
  };
}