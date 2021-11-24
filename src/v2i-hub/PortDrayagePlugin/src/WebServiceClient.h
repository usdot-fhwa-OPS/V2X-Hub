/**
 * Copyright (C) 2019 LEIDOS.
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
#include <iostream>
#include <OAIDefaultApi.h>
#include "PluginLog.h"
#include <QEventLoop>
#include <OAIHelpers.h>
#include <QTimer>
#include <OAIContainerActionStatus.h>

using namespace OpenAPI;
using namespace tmx::utils;

// Default values for no arg constructor
static CONSTEXPR const char *DEF_HOST = "127.0.0.1";
static CONSTEXPR const uint16_t DEF_PORT = 8090;
static CONSTEXPR const uint16_t DEF_POLLING_FREQ = 5;
static CONSTEXPR const bool DEF_SECURITY = false;

/**
 * WebService REST Client using OpenAPI codegen library pdclient found under V2X-Hub/ext/pdclient. Contains several method to
 * send POST requests for loading, unloading, and inspection actions and poll the action status.
 * 
 * @author Paul Bourelly
 */
class WebServiceClient : public QObject
{
    Q_OBJECT
public slots:

private:
    // Stored in Seconds
    uint16_t polling_frequency;


    // OAIDefaultApi pointer
    std::shared_ptr<OAIDefaultApi> api;
    std::shared_ptr<OAIContainerActionStatus> loading_status;
    std::shared_ptr<OAIContainerActionStatus> unloading_status;
    std::shared_ptr<OAIInspectionStatus> inspection_status;
    /**
     * Method to poll the status of a loading action with a given action id.
     * 
     * @param action_id of the loading action to be polled
     */ 
    void pollLoadingAction(QString action_id);

    /**
     * Method to poll the status of a unloading action with a given action id.
     * 
     * @param action_id of the unloading action to be polled 
     */ 
    void pollUnloadingAction(QString action_id);

    /**
     * Method to poll the status of a inspection with a given action id.
     * 
     * @param action_id of the inspection to be polled
     * @return 0 if inspection is passed and 1 if further inspection at the holding area is requested
     */ 
    int pollInspectionAction(QString action_id);

    /**
     * Method to initialize server configuration and polling frequency
     * 
     * @param host string host name of server
     * @param port uint16_t port of server
     * @param secure bool flag set to true for using HTTPS
     * @param polling_frequency 
     */ 
    void initialize(const std::string &host, uint16_t port, bool secure , uint16_t polling_frequency);


public:

    /**
     * Constructor without parameters
     */
    WebServiceClient();

    /**
     * Constructor for WebServiceClient
     * 
     * @param host string webservice host URL
     * @param port uint8_t webservice port
     * @param secure boolean flag set to true when using HTTPS
     * @param int polling frequency in seconds for action status
     *  
     */ 
    WebServiceClient(const std::string &host, uint16_t port, bool secure , uint16_t polling_frequency );

    /**
     * Method to request a loading action. Sends a HTTP POST call to the loading endpoint of the PortDrayage Webservice and then
     * polls the status of the request every 5 seconds. Method will exit once loading action is completed.
     * 
     * @param vehicle_id static unique identifier for vehicle
     * @param container_id static unique identifier for container
     * @param action_id static unique identifier for action 
     */
    void request_loading_action(const std::string &vehicle_id, const std::string &container_id, const std::string &action_id);
    /**
     * Method to request an unloading action. Sends a HTTP POST call to the unloading endpoint of the PortDrayage Webservice and then
     * polls the status of the request every 5 seconds. Method will exit once unloading action is completed.
     * 
     * @param vehicle_id static unique identifier for the vehicle
     * @param container_id static unique identifier for the container
     * @param action_id static unique identifier for the action 
     */
    void request_unloading_action(const std::string &vehicle_id, const std::string &container_id, const std::string &action_id);
    /**
     * Method to request an inspection action. Sends a HTTP POST call to the inspection endpoint of the PortDrayage Webservice and then
     * polls the status of the request every 5 seconds. Method will exit once inspection action is completed or operator indicates the 
     * vehicle requires further inspection at the Holding area
     * 
     * @param vehicle_id static unique identifier for the vehicle
     * @param container_id static unique identifier for container
     * @param action_id static unique identifier for the action
     */
    int request_inspection(const std::string &vehicle_id, const std::string &container_id, const std::string &action_id);
    /**
     * Method request further inspection at the Holding area. Sends a HTTP POST call to inspection holding endpoint of the PortDrayage Webservice
     * and then poll the status of the request every 5 seconds. Method will exit once inspection action is completed.
     * 
     * @param action_id static unique identifier for action
     */
    void request_holding(const std::string &action_id);
    

};
