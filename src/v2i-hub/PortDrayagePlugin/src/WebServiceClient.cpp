#include "WebServiceClient.h"


WebServiceClient::WebServiceClient() {
    // 5s
    polling_frequency = 5;
    api = new OAIDefaultApi(0);
    api->setNewServerForAllOperations(
        QUrl("http://127.0.0.1:8090"),
        QString::fromStdString("Unsecured hosting for development"),
        QMap<QString, OAIServerVariable>()
    );
}

WebServiceClient::WebServiceClient(std::string host, uint8_t port, bool secure , uint16_t polling_frequency ) {
    this->polling_frequency = polling_frequency;
    // OAIDefaultApi api;

    // Create URL
    QUrl url;
    url.setHost(QString::fromStdString(host));
    url.setPort(port);
    if ( secure ) {
        url.setScheme(QString::fromStdString("https"));
    }
    else {
        url.setScheme(QString::fromStdString("http"));

    }
    api = new OAIDefaultApi(0);
    api->setNewServerForAllOperations(
        url,
        QString::fromStdString("V2X-Hub Configured PortDrayage WebService"),
        QMap<QString, OAIServerVariable>()
    );
}

    
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
    QEventLoop loop;

     // Call back for POST /loading/
    connect(api, &OAIDefaultApi::loadingPostSignal, [&]() {
        FILE_LOG(logERROR) << "Success /loading POST";
        loop.quit();
    });
    // Error call back for POST /loading/
    connect(api, &OAIDefaultApi::loadingPostSignalE, [&](QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /loading POST : " << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure /loading POST : " << error_code;

        loop.quit();
    });


    req.setVehicleId( QString::fromStdString( vehicle_id ) ) ;
    req.setContainerId( QString::fromStdString( container_id ) );
    req.setActionId( QString::fromStdString( action_id ) );
    FILE_LOG(logERROR) << "Sending loading request : " << req.asJson().toStdString();

    api->loadingPost( req );
    QTimer::singleShot(0, &loop, &QEventLoop::quit);
    loop.exec();

    // Poll loading action until complete
    pollLoadingAction( req.getActionId() );
   
}

void WebServiceClient::request_unloading_action(std::string vehicle_id, std::string container_id, std::string action_id) {
    OAIContainerRequest req;
    QEventLoop loop;

    // Call back for POST /unloading/
    connect(api, &OAIDefaultApi::unloadingPostSignal, [&]() {
        FILE_LOG(logERROR) << "Success /unloading POST";
        loop.quit();
    });
    // Error call back for POST /unloading/
    connect(api, &OAIDefaultApi::unloadingPostSignalE, [&](QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /unloading POST : " << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure /unloading POST : " << error_code;

        loop.quit();
    });

    req.setVehicleId( QString::fromStdString( vehicle_id ) ) ;
    req.setContainerId( QString::fromStdString( container_id ) );
    req.setActionId( QString::fromStdString( action_id ) );
    api->unloadingPost( req );

    QTimer::singleShot(0, &loop, &QEventLoop::quit);
    loop.exec();

    pollUnloadingAction( req.getActionId() );

}

int WebServiceClient::request_inspection(std::string vehicle_id, std::string container_id, std::string action_id ) {
    OAIInspectionRequest req;
    // OAIDefaultApi api;
    QEventLoop loop;

     // Call back for POST /inspection/
    connect(api, &OAIDefaultApi::inspectionPostSignal, [&]() {
        FILE_LOG(logERROR) << "Success /inspection POST";
        loop.quit();
    });
    // Error call back for POST /inspection/
    connect(api, &OAIDefaultApi::inspectionPostSignalE, [&](QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /inspection POST : " << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure /inspection POST : " << error_code;

        loop.quit();
    });

    req.setVehicleId( QString::fromStdString( vehicle_id ) ) ;
    req.setContainerId( QString::fromStdString( container_id ) );
    req.setActionId( QString::fromStdString( action_id ) );
    api->inspectionPost( req );

    QTimer::singleShot(0, &loop, &QEventLoop::quit);
    loop.exec();

    return pollInspectionAction( req.getActionId() );
}

void WebServiceClient::request_holding( std::string action_id ) {
    QEventLoop loop;

        // Call back for POST /inspection/
    connect(api, &OAIDefaultApi::inspectionHoldingActionIdPostSignal, [&]() {
        FILE_LOG(logERROR) << "Success /inspection/holding/{action_id} POST";
        loop.quit();
    });
    // Error call back for POST /inspection/
    connect(api, &OAIDefaultApi::inspectionHoldingActionIdPostSignalE, [&](QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /inspection/holding/{action_id} POST : " << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure /inspection/holding/{action_id} POST : " << error_code;

        loop.quit();
    });

    api->inspectionHoldingActionIdPost( QString::fromStdString(action_id) );
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    pollInspectionAction( QString::fromStdString( action_id ) );

}

void WebServiceClient::pollLoadingAction( QString action_id ) {
    QEventLoop loop;

     // Call back for Get /loading/{action_id}
    connect(api, &OAIDefaultApi::loadingActionIdGetSignal, [&](OAIContainerActionStatus loading_action) {
        FILE_LOG(logERROR) << "Success /loading/{action_id} GET : " << loading_action.asJson().toStdString();
        current_loading_action = loading_action; 
        loop.quit();
    });
    // Error call back for Get /loading/{action_id}
    connect(api, &OAIDefaultApi::loadingActionIdGetSignalE, 
        [&](OAIContainerActionStatus loading_action , QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure loading/{action_id} GET :" << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure loading/{action_id} POST : " << error_code;

        loop.quit();
    });


    api->loadingActionIdGet( action_id );
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();
    while ( current_loading_action.getStatus() != QString::fromStdString( "LOADED") ) {
        api->loadingActionIdGet( action_id );

        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();
        // usleep coversion from seconds to microseconds
        usleep( polling_frequency * 1e6 );
    }

} 

void WebServiceClient::pollUnloadingAction( QString action_id) {
    QEventLoop loop;

     // Call back for Get /unloading/{action_id}
    connect(api, &OAIDefaultApi::unloadingActionIdGetSignal, [&](OAIContainerActionStatus unloading_action) {
        FILE_LOG(logERROR) << "Success /unloading/{action_id} GET : " << unloading_action.asJson().toStdString();
        current_unloading_action = unloading_action; 
        
        loop.quit();
    });
    // Error call back for Get /unloading/{action_id}
    connect(api, &OAIDefaultApi::unloadingActionIdGetSignalE, 
        [&](OAIContainerActionStatus unloading_action , QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure unloading/{action_id} GET :" << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure unloading/{action_id} GET : " << error_code;

        loop.quit();
    });

    api->unloadingActionIdGet( action_id );
    QTimer::singleShot(0, &loop, &QEventLoop::quit);
    loop.exec();
    while ( current_unloading_action.getStatus() != QString::fromStdString( "UNLOADED") ) {
        api->unloadingActionIdGet( action_id );

        QTimer::singleShot(0, &loop, &QEventLoop::quit);
        loop.exec();
        
        // usleep coversion from seconds to microseconds
        usleep( polling_frequency * 1e6 );


    }
}

int WebServiceClient::pollInspectionAction( QString action_id ) {
    QEventLoop loop;

    // Call back for GET /inspection/{action_id}
    connect(api, &OAIDefaultApi::inspectionActionIdGetSignal, [&](OAIInspectionStatus inspection) {
            current_inspection = inspection; 
            FILE_LOG(logERROR) << "Success /inspection/{action_id} GET : " << current_inspection.asJson().toStdString() ;
            loop.quit();
    });
    // Error call back for /inspection/{action_id}
    connect(api, &OAIDefaultApi::inspectionActionIdGetSignalE, 
        [&](OAIInspectionStatus inspection , QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /inspection/{action_id} GET : " << error_str.toStdString();
        FILE_LOG(logERROR) << "Failure /inspection/{action_id} GET : " << error_code;
        loop.quit();
    });

    api->inspectionActionIdGet( action_id );
    QTimer::singleShot(0, &loop, &QEventLoop::quit);
    loop.exec();


    while ( true ) {
        api->inspectionActionIdGet( action_id );

        QTimer::singleShot(0, &loop, &QEventLoop::quit);
        loop.exec();
        if (current_inspection.getStatus() == QString::fromStdString( "PASSED")){
            return 0;
        }
        else if (current_inspection.getStatus() == QString::fromStdString( "PROCEED_TO_HOLDING")) {
            return 1;
        }
        // usleep coversion from seconds to microseconds
        usleep( polling_frequency * 1e6 );

    }
}

    
