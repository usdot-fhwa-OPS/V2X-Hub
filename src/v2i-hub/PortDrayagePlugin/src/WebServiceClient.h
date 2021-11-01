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


class WebServiceClient : public QObject {
    Q_OBJECT
    public slots:

    private: 
        OAIContainerActionStatus *current_loading_action;
        OAIContainerActionStatus *current_unloading_action;
        OAIInspectionStatus *current_inspection;

    
    public : 
        void setup_callbacks();
        OAIContainerActionStatus get_current_loading_action();
        OAIContainerActionStatus get_current_unloading_action();
        OAIInspectionStatus get_current_inspection();
        void request_loading_action(std::string vehicle_id, std::string container_id, std::string action_id);
    
};
