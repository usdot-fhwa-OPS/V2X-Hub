#include "WebServiceClient.h"

    
OAIContainerActionStatus WebServiceClient::get_current_unloading_action() {
    return current_unloading_action;
}

OAIContainerActionStatus WebServiceClient::get_current_loading_action() {
    return current_loading_action;
}

OAIInspectionStatus WebServiceClient::get_current_inspection() {
    return current_inspection;
}

void WebServiceClient::request_loading_action(std::string vehicle_id, std::string container_id, std::string action_id) {
    OAIContainerRequest req;
    OAIDefaultApi api;
    QEventLoop loop;
    // Call back for Get /loading/{action_id}
    connect(&api, &OAIDefaultApi::loadingActionIdGetSignal, [&](OAIContainerActionStatus loading_action) {
        FILE_LOG(logERROR) << "Success /loading/{action_id} GET : " << loading_action.asJson().toStdString();
        current_loading_action = loading_action; 
        
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


    api.loadingActionIdGet( req.getActionId() );
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    while ( current_loading_action.getStatus() != QString::fromStdString( "LOADED") ) {
        api.loadingActionIdGet( req.getActionId() );
        FILE_LOG(logERROR) << "Current action : " << current_loading_action.asJson().toStdString();
        FILE_LOG(logERROR) << "Success /loading/{action_id} GET : " << current_loading_action.getStatus().toStdString();

        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        usleep( 5000000 );
        loop.exec();
    }
    
}

void WebServiceClient::request_unloading_action(std::string vehicle_id, std::string container_id, std::string action_id) {
    OAIContainerRequest req;
    OAIDefaultApi api;
    QEventLoop loop;
    // Call back for Get /unloading/{action_id}
    connect(&api, &OAIDefaultApi::unloadingActionIdGetSignal, [&](OAIContainerActionStatus unloading_action) {
        FILE_LOG(logERROR) << "Success /unloading/{action_id} GET : " << unloading_action.asJson().toStdString();
        current_unloading_action = unloading_action; 
        
        loop.quit();
    });
    // Error call back for Get /unloading/{action_id}
    connect(&api, &OAIDefaultApi::unloadingActionIdGetSignalE, 
        [&](OAIContainerActionStatus unloading_action , QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure unloading/{action_id} GET :" << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure unloading/{action_id} POST : " << error_code;

        loop.quit();
    });
    // Call back for POST /unloading/
    connect(&api, &OAIDefaultApi::unloadingPostSignal, [&]() {
        FILE_LOG(logERROR) << "Success /unloading POST";
        loop.quit();
    });
    // Error call back for POST /unloading/
    connect(&api, &OAIDefaultApi::unloadingPostSignalE, [&](QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /unloading POST : " << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure /unloading POST : " << error_code;

        loop.quit();
    });

    req.setVehicleId( QString::fromStdString( vehicle_id ) ) ;
    req.setContainerId( QString::fromStdString( container_id ) );
    req.setActionId( QString::fromStdString( action_id ) );
    api.unloadingPost( req );

    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();


    api.unloadingActionIdGet( req.getActionId() );
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    while ( current_unloading_action.getStatus() != QString::fromStdString( "UNLOADED") ) {
        api.unloadingActionIdGet( req.getActionId() );
        FILE_LOG(logERROR) << "Current action : " << current_unloading_action.asJson().toStdString();
        FILE_LOG(logERROR) << "Success /unloading/{action_id} GET : " << current_unloading_action.getStatus().toStdString();

        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        usleep( 5000000 );
        loop.exec();
    }
}

int WebServiceClient::request_inspection(std::string vehicle_id, std::string container_id, std::string action_id ) {
    OAIInspectionRequest req;
    OAIDefaultApi api;
    QEventLoop loop;
    connect(&api, &OAIDefaultApi::inspectionActionIdGetSignal, [&](OAIInspectionStatus inspection) {
            FILE_LOG(logERROR) << "Success";
            current_inspection = inspection; 
            loop.quit();
    });

    connect(&api, &OAIDefaultApi::inspectionActionIdGetSignalE, 
        [&](OAIInspectionStatus inspection , QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /inspection/{action_id} GET : " << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure /inspection/{action_id} GET : " << error_code;
        loop.quit();
    });

    // Call back for POST /inspection/
    connect(&api, &OAIDefaultApi::inspectionPostSignal, [&]() {
        FILE_LOG(logERROR) << "Success /inspection POST";
        loop.quit();
    });
    // Error call back for POST /inspection/
    connect(&api, &OAIDefaultApi::inspectionPostSignalE, [&](QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /inspection POST : " << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure /inspection POST : " << error_code;

        loop.quit();
    });

    req.setVehicleId( QString::fromStdString( vehicle_id ) ) ;
    req.setContainerId( QString::fromStdString( container_id ) );
    req.setActionId( QString::fromStdString( action_id ) );
    api.inspectionPost( req );

    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();


    api.inspectionActionIdGet( req.getActionId() );
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    while ( true ) {
        api.inspectionActionIdGet( req.getActionId() );
        FILE_LOG(logERROR) << "Current inspection : " << current_inspection.asJson().toStdString();
        FILE_LOG(logERROR) << "Success /inspection/{action_id} GET : " << current_inspection.getStatus().toStdString();

        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        usleep( 5000000 );
        loop.exec();
        if (current_inspection.getStatus() == QString::fromStdString( "PASSED")){
            return 0;
        }
        else if (current_inspection.getStatus() == QString::fromStdString( "PROCEED_TO_HOLDING")) {
            return 1;
        }
    }
    FILE_LOG(logERROR) << "Something went wrong!";
    return 1;
}

void WebServiceClient::request_holding( std::string action_id ) {
    OAIDefaultApi api;
    QEventLoop loop;
    connect(&api, &OAIDefaultApi::inspectionActionIdGetSignal, [&](OAIInspectionStatus inspection) {
            FILE_LOG(logERROR) << "Success";
            current_inspection = inspection; 
            loop.quit();
    });

    connect(&api, &OAIDefaultApi::inspectionActionIdGetSignalE, 
        [&](OAIInspectionStatus inspection , QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /inspection/{action_id} GET : " << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure /inspection/{action_id} GET : " << error_code;
        loop.quit();
    });

    // Call back for POST /inspection/
    connect(&api, &OAIDefaultApi::inspectionHoldingActionIdPostSignal, [&]() {
        FILE_LOG(logERROR) << "Success /inspection/holding/{action_id} POST";
        loop.quit();
    });
    // Error call back for POST /inspection/
    connect(&api, &OAIDefaultApi::inspectionHoldingActionIdPostSignalE, [&](QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /inspection/holding/{action_id} POST : " << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure /inspection/holding/{action_id} POST : " << error_code;

        loop.quit();
    });
    api.inspectionHoldingActionIdPost( QString::fromStdString(action_id) );

    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();


    api.inspectionActionIdGet( QString::fromStdString(action_id) );
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    while ( current_inspection.getStatus() != QString::fromStdString( "PASSED") ) {
        api.inspectionActionIdGet( QString::fromStdString(action_id) );
        FILE_LOG(logERROR) << "Current inspection : " << current_inspection.asJson().toStdString();
        FILE_LOG(logERROR) << "Success /inspection/{action_id} GET : " << current_inspection.getStatus().toStdString();

        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        usleep( 5000000 );
        loop.exec();
    }

}

    
