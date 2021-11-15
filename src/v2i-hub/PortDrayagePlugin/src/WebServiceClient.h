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
    OAIDefaultApi *api;

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
    WebServiceClient(std::string host, uint16_t port, bool secure , uint16_t polling_frequency );

    /**
     * Method to request a loading action. Sends a HTTP POST call to the loading endpoint of the PortDrayage Webservice and then
     * polls the status of the request every 5 seconds. Method will exit once loading action is completed.
     * 
     * @param vehicle_id static unique identifier for vehicle
     * @param container_id static unique identifier for container
     * @param action_id static unique identifier for action 
     */
    void request_loading_action(std::string vehicle_id, std::string container_id, std::string action_id);
    /**
     * Method to request an unloading action. Sends a HTTP POST call to the unloading endpoint of the PortDrayage Webservice and then
     * polls the status of the request every 5 seconds. Method will exit once unloading action is completed.
     * 
     * @param vehicle_id static unique identifier for the vehicle
     * @param container_id static unique identifier for the container
     * @param action_id static unique identifier for the action 
     */
    void request_unloading_action(std::string vehicle_id, std::string container_id, std::string action_id);
    /**
     * Method to request an inspection action. Sends a HTTP POST call to the inspection endpoint of the PortDrayage Webservice and then
     * polls the status of the request every 5 seconds. Method will exit once inspection action is completed or operator indicates the 
     * vehicle requires further inspection at the Holding area
     * 
     * @param vehicle_id static unique identifier for the vehicle
     * @param container_id static unique identifier for container
     * @param action_id static unique identifier for the action
     */
    int request_inspection(std::string vehicle_id, std::string container_id, std::string action_id);
    /**
     * Method request further inspection at the Holding area. Sends a HTTP POST call to inspection holding endpoint of the PortDrayage Webservice
     * and then poll the status of the request every 5 seconds. Method will exit once inspection action is completed.
     * 
     * @param action_id static unique identifier for action
     */
    void request_holding(std::string action_id);
    

};
