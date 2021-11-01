#include "WebServiceClient.h"

    
void WebServiceClient::setup_callbacks() {
    // OAIDefaultApi api;

    // QEventLoop loop;

    
    
    // connect(&api, &OAIDefaultApi::unloadingActionIdGetSignal, [&](OAIContainerActionStatus unloading_action) {
    //     FILE_LOG(logERROR) << "Success";
    //     current_unloading_action = &unloading_action; 
    //     loop.quit();
    // });

    // connect(&api, &OAIDefaultApi::inspectionActionIdGetSignal, [&](OAIInspectionStatus inspection) {
    //     FILE_LOG(logERROR) << "Success";
    //     current_inspection = &inspection; 
    //     loop.quit();
    // });

    

    // connect(&api, &OAIDefaultApi::unloadingActionIdGetSignalE, 
    //     [&](OAIContainerActionStatus unloading_action , QNetworkReply::NetworkError, QString error_str) {
    //     FILE_LOG(logERROR) << "Failure : " << error_str.toStdString();
    //     loop.quit();
    // });

    //  connect(&api, &OAIDefaultApi::inspectionActionIdGetSignalE, 
    //     [&](OAIInspectionStatus inspection , QNetworkReply::NetworkError, QString error_str) {
    //     FILE_LOG(logERROR) << "Failure : " << error_str.toStdString();
    //     loop.quit();
    // });
    

    // QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    // loop.exec();
}

OAIContainerActionStatus WebServiceClient::get_current_unloading_action() {
    return *current_unloading_action;
}

OAIContainerActionStatus WebServiceClient::get_current_loading_action() {
    return *current_loading_action;
}

OAIInspectionStatus WebServiceClient::get_current_inspection() {
    return *current_inspection;
}

void WebServiceClient::request_loading_action(std::string vehicle_id, std::string container_id, std::string action_id) {
    OAIContainerRequest req;
    OAIDefaultApi api;
    QEventLoop loop;
    // Call back for Get /loading/{action_id}
    connect(&api, &OAIDefaultApi::loadingActionIdGetSignal, [&](OAIContainerActionStatus loading_action) {
        FILE_LOG(logERROR) << "Success /loading/{action_id} GET";
        current_loading_action = &loading_action; 
        loop.quit();
    });
    // Error call back for Get /loading/{action_id}
    connect(&api, &OAIDefaultApi::loadingActionIdGetSignalE, 
        [&](OAIContainerActionStatus loading_action , QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure loading/{action_id} GET :" << error_str.toStdString();
         FILE_LOG(logERROR) << "Failure loading/{action_id} POST : " << error_code;

        loop.quit();
    });
    // Call back for POST /loading/
    connect(&api, &OAIDefaultApi::loadingPostSignal, [&]() {
        FILE_LOG(logERROR) << "Success /loading POST";
        loop.quit();
    });
    // Error call back for POST /loading/
    connect(&api, &OAIDefaultApi::loadingPostSignalE, [&](QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /loading POST : " << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure /loading POST : " << error_code;

        loop.quit();
    });

    req.setVehicleId( QString::fromStdString( vehicle_id ) ) ;
    req.setContainerId( QString::fromStdString( container_id ) );
    req.setActionId( QString::fromStdString( action_id ) );
    api.loadingPost( req );

    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();
    
}
