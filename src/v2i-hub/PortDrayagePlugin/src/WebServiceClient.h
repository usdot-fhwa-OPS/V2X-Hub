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
 * 
 */
class WebServiceClient : public QObject
{
    Q_OBJECT
public slots:

private:
    OAIContainerActionStatus current_loading_action;
    OAIContainerActionStatus current_unloading_action;
    OAIInspectionStatus current_inspection;

public:
    OAIContainerActionStatus get_current_loading_action();
    OAIContainerActionStatus get_current_unloading_action();
    OAIInspectionStatus get_current_inspection();
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
