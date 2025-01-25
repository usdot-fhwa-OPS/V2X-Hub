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
#include "WebSocketServer.h"

namespace CDA1TenthPlugin{

  WebSocketServer::WebSocketServer(){
    lws_protocols protocols[2] = {
      {
        "websocket-protocol",
        webSocketCallback,
        PER_SESSION_USER_DATA_SIZE, 
        MAX_FRAME_SIZE
      },
      {NULL, NULL, 0, 0}
    };

    memset(&context_info, 0, sizeof(context_info));
    context_info.port = PORT;
    context_info.protocols = protocols;
    context_info.gid = -1;
    context_info.uid = -1;
    context_info.user = this;
    context = lws_create_context(&context_info);
    if(!context){
      PLOG(logERROR) << "Failed to create websocket context!";
    }
  };

  WebSocketServer::~WebSocketServer(){
    if(context){
      lws_context_destroy(context);
    }
  };
  
  void WebSocketServer::removeLWSClients(lws* wsi){
    lock_guard<mutex> lock(clients_mutex);
    clients.erase(std::remove(clients.begin(), clients.end(), wsi), clients.end());
    PLOG(logDEBUG1) << "Client disconnected. Total clients: " << clients.size();
  };

  void WebSocketServer::addLWSClients(lws* wsi){
    lock_guard<mutex> lock(clients_mutex);
    clients.push_back(wsi);
    PLOG(logDEBUG1) << "Client connected. Total clients: " << clients.size();
  };

  int WebSocketServer::webSocketCallback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len){
    WebSocketServer* server = static_cast<WebSocketServer*>(lws_context_user(lws_get_context(wsi)));
    if (!server) {
      PLOG(logERROR) << "Server is null" << std::endl;
      return 0; 
    }
    switch (reason)
    {
    case LWS_CALLBACK_ESTABLISHED:
       PLOG(logDEBUG) << "Connection established" << std::endl;
       server->addLWSClients(wsi);
      break;
    case LWS_CALLBACK_RECEIVE:
       PLOG(logDEBUG2) << "Data received: " << std::string((char*)in, len) << std::endl;
      break;
    case LWS_CALLBACK_CLOSED:
       PLOG(logDEBUG) << "Connection closed" << std::endl;
        server->removeLWSClients(wsi);
      break;
    default:
      break;
    }
    return 0;
  }

  void WebSocketServer::run(){
    setRunning(true); 
     while(isRunning()){
        PLOG(logDEBUG4) << "websocket is running..." << std::endl;  
        lws_service(context, PULL_PERIOD);
        sendQueuedMessagesToAllClients();
      }
  }

  bool WebSocketServer::isRunning(){
    return running.load();
  }

  void WebSocketServer::setRunning(bool value){
    running.store(value);
  }

  void WebSocketServer::addMessage(std::string msg){
    lock_guard<mutex> lock(message_queue_mutex);
    message_queue.push_back(msg);
  }

  void WebSocketServer::sendMessage(lws* wsi, const string& message) const{
    if(!wsi){
      PLOG(logERROR) << "A client does not exist!" <<std::endl;
      return;
    }

    unsigned char* out = (unsigned char*)malloc(LWS_SEND_BUFFER_PRE_PADDING + message.length() + LWS_SEND_BUFFER_POST_PADDING);
    if (!out) {
        PLOG(logERROR) << "Failed to allocate memory for message" << std::endl;
        return;
    }
    memcpy(out + LWS_SEND_BUFFER_PRE_PADDING, message.c_str(), message.length());
    int n = lws_write(wsi, out + LWS_SEND_BUFFER_PRE_PADDING, message.length(), LWS_WRITE_TEXT);

    free(out);

    if (n < 0) {
        PLOG(logERROR) << "lws_write() failed: " << n << std::endl;
    } else if (n < (int)message.length()) {
        PLOG(logERROR) << "Partial write: " << n << " of " << message.length() << std::endl;
    }

    PLOG(logDEBUG3) << "Sent a queued message to a connected client! Message content: " << message << std::endl;
  }

  void WebSocketServer::sendMsgToAllClients(const std::string& msg){
    lock_guard<mutex> lock(clients_mutex);
    for(auto client : clients){  
        sendMessage(client, msg);
    }
  };

  void WebSocketServer::sendQueuedMessagesToAllClients(){
    lock_guard<mutex> lock(message_queue_mutex);
    if(message_queue.size() > 0){
      PLOG(logDEBUG2) << "Sending messages. Current client size: " << clients.size() << ", current message queue size: " << message_queue.size() << std::endl;
      for(auto msg : message_queue){
        sendMsgToAllClients(msg);
      }
      message_queue.clear(); 
      PLOG(logDEBUG2) << "Messages sent! Current client size: " << clients.size() << ", current message queue size: " << message_queue.size() << std::endl;        
    }
   }

}