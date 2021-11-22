#include "WebServiceClient.h"


WebServiceClient::WebServiceClient() {
    initialize("127.0.0.1", 8090, false, 5);
}

WebServiceClient::WebServiceClient(std::string host, uint16_t port, bool secure , uint16_t polling_frequency ) {
    initialize(host, port, secure, polling_frequency);
}
void WebServiceClient::initialize(std::string host, uint16_t port, bool secure , uint16_t polling_frequency) {
    this->polling_frequency = polling_frequency;

    // Create URL
    std::unique_ptr<QUrl> url( new QUrl());
    url.get()->setHost(QString::fromStdString(host));
    url.get()->setPort(port);
    if ( secure ) {
        url.get()->setScheme(QString::fromStdString("https"));
    }
    else {
        url.get()->setScheme(QString::fromStdString("http"));

    }
    FILE_LOG(logINFO) << "Setting API URL as " << url.get()->toString().toStdString() << std::endl;
    // Initialize API
    api = std::make_shared<OAIDefaultApi>(0);
    // Setup server config
    api.get()->setNewServerForAllOperations(
        *url.get(),
        QString::fromStdString("V2X-Hub Configured PortDrayage WebService"),
        QMap<QString, OAIServerVariable>()
    );
    
}

void WebServiceClient::request_loading_action(std::string vehicle_id, std::string container_id, std::string action_id) {
    std::unique_ptr<OAIContainerRequest> req(new OAIContainerRequest ());
    std::unique_ptr<QEventLoop> loop( new QEventLoop());

    // Disconnect duplicate signals
    disconnect(api.get(), &OAIDefaultApi::loadingPostSignal, nullptr, nullptr);
    disconnect(api.get(), &OAIDefaultApi::loadingPostSignalE, nullptr, nullptr);

    // Call back for POST /loading/
    connect(api.get(), &OAIDefaultApi::loadingPostSignal, this, [&]() {
        FILE_LOG(logINFO) << "Success /loading POST";
        loop->quit();
    });
    // Error call back for POST /loading/
    connect(api.get(), &OAIDefaultApi::loadingPostSignalE, this, [&](QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /loading POST : " << error_str.toStdString();
        FILE_LOG(logERROR) << error_code;
        loop->quit();
    });

    // Setup request
    req->setVehicleId( QString::fromStdString( vehicle_id ) ) ;
    req->setContainerId( QString::fromStdString( container_id ) );
    req->setActionId( QString::fromStdString( action_id ) );

    FILE_LOG(logINFO) << "Sending loading request : " << req->asJson().toStdString();
    api->loadingPost( *req.get() );
    loop->exec();      
    // Poll loading action until complete
    pollLoadingAction( req->getActionId() );
 
  
}

void WebServiceClient::request_unloading_action(std::string vehicle_id, std::string container_id, std::string action_id) {
    std::unique_ptr<OAIContainerRequest> req(new OAIContainerRequest ());
    std::unique_ptr<QEventLoop> loop( new QEventLoop());

    // Disconnect duplicate signals
    disconnect(api.get(), &OAIDefaultApi::unloadingPostSignal, nullptr, nullptr);
    disconnect(api.get(), &OAIDefaultApi::unloadingPostSignalE, nullptr, nullptr);

    // Call back for POST /unloading/
    connect(api.get(), &OAIDefaultApi::unloadingPostSignal, this, [&]() {
        FILE_LOG(logINFO) << "Success /unloading POST";
        loop->quit();
    });
    // Error call back for POST /unloading/
    connect(api.get(), &OAIDefaultApi::unloadingPostSignalE, this, [&](QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /unloading POST : " << error_str.toStdString();
        FILE_LOG(logERROR) << error_code;
        loop->quit();
    });

    // Setup request
    req->setVehicleId( QString::fromStdString( vehicle_id ) ) ;
    req->setContainerId( QString::fromStdString( container_id ) );
    req->setActionId( QString::fromStdString( action_id ) );

    FILE_LOG(logINFO) << "Sending unloading request : " << req->asJson().toStdString();
    api->unloadingPost( *req.get() );
    loop->exec();

    // Polling unloading action until complete 
    pollUnloadingAction( req->getActionId() );

}

int WebServiceClient::request_inspection(std::string vehicle_id, std::string container_id, std::string action_id ) {
    std::unique_ptr<OAIInspectionRequest> req(new OAIInspectionRequest());
    std::unique_ptr<QEventLoop> loop( new QEventLoop());

    // Disconnect all duplicate signals
    disconnect(api.get(),&OAIDefaultApi::inspectionPostSignal, nullptr, nullptr );
    disconnect(api.get(),&OAIDefaultApi::inspectionPostSignalE, nullptr, nullptr );

    // Call back for POST /inspection/
    connect(api.get(), &OAIDefaultApi::inspectionPostSignal, this, [&]() {
        FILE_LOG(logINFO) << "Success /inspection POST";
        loop->quit();
    });
    // Error call back for POST /inspection/
    connect(api.get(), &OAIDefaultApi::inspectionPostSignalE, this, [&](QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /inspection POST : " << error_str.toStdString();
        FILE_LOG(logERROR) << error_code;
        loop->quit();
    });

    // Setup request
    req->setVehicleId( QString::fromStdString( vehicle_id ) ) ;
    req->setContainerId( QString::fromStdString( container_id ) );
    req->setActionId( QString::fromStdString( action_id ) );

    FILE_LOG(logINFO) << "Sending inspection request : " << req->asJson().toStdString();
    api->inspectionPost( *req.get() );
    loop->exec();

    // Poll inspection status until complete or proceed to holding
    return pollInspectionAction( req->getActionId() );
}

void WebServiceClient::request_holding( std::string action_id ) {
    std::unique_ptr<QEventLoop> loop( new QEventLoop());

    // Disconnect all duplicate signals
    disconnect(api.get(), &OAIDefaultApi::inspectionHoldingActionIdPostSignal, nullptr, nullptr);
    disconnect(api.get(), &OAIDefaultApi::inspectionHoldingActionIdPostSignalE, nullptr, nullptr);

    // Call back for POST /inspection/
    connect(api.get(), &OAIDefaultApi::inspectionHoldingActionIdPostSignal, [&]() {
        FILE_LOG(logINFO) << "Success /inspection/holding/{action_id} POST";
        loop->quit();
    });
    // Error call back for POST /inspection/
    connect(api.get(), &OAIDefaultApi::inspectionHoldingActionIdPostSignalE, [&](QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /inspection/holding/{action_id} POST : " << error_str.toStdString();
        FILE_LOG(logERROR) << error_code;
        loop->quit();
    });
    FILE_LOG(logINFO) << "Sending holding request for action_id : " << action_id << std::endl;
    api.get()->inspectionHoldingActionIdPost( QString::fromStdString(action_id) );
    loop->exec();
    // Poll inspection action until complete
    pollInspectionAction( QString::fromStdString( action_id ) );

}

void WebServiceClient::pollLoadingAction( QString action_id ) {
    FILE_LOG(logDEBUG) << "Starting loading action Polling";
    std::unique_ptr<QEventLoop> loop( new QEventLoop());
    bool badResponse = true;
    // Disconnect all duplicate signals
    disconnect(api.get(), &OAIDefaultApi::loadingActionIdGetSignal, nullptr, nullptr);
    disconnect(api.get(), &OAIDefaultApi::loadingActionIdGetSignalE, nullptr, nullptr);

    // Call back for Get /loading/{action_id}
    connect(api.get(), &OAIDefaultApi::loadingActionIdGetSignal, this,  [&](OAIContainerActionStatus loading_action) {
        loading_status.reset( new OAIContainerActionStatus( loading_action.asJson( ) ) );
        FILE_LOG(logINFO) << "Success /loading/{action_id} GET : " << loading_status->asJson().toStdString();
        badResponse = false;
        loop->quit();
    });
    // Error call back for Get /loading/{action_id}
    connect(api.get(), &OAIDefaultApi::loadingActionIdGetSignalE, this,
        [&](OAIContainerActionStatus loading_action , QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure loading/{action_id} GET :" << error_str.toStdString();
        FILE_LOG(logERROR) << error_code;
        badResponse = true;
        loop->quit();
    });
    // Flag to continue polling until receiving a non error response from server
    do  {
        api->loadingActionIdGet( action_id );
        loop->exec();
        // usleep coversion from seconds to microseconds
        usleep( polling_frequency * 1e6 );
    }
    while ( badResponse || loading_status->getStatus() != QString::fromStdString( "LOADED") ) ;

} 

void WebServiceClient::pollUnloadingAction( QString action_id) {
    FILE_LOG(logDEBUG) << "Starting unloading action Polling";
    std::unique_ptr<QEventLoop> loop( new QEventLoop());

    bool badResponse = true;
    // Disconnect all duplicate signals
    disconnect(api.get(), &OAIDefaultApi::unloadingActionIdGetSignal, nullptr, nullptr);
    disconnect(api.get(), &OAIDefaultApi::unloadingActionIdGetSignalE, nullptr, nullptr);
     // Call back for Get /unloading/{action_id}
    connect(api.get(), &OAIDefaultApi::unloadingActionIdGetSignal, this, [&](OAIContainerActionStatus unloading_action) {
        unloading_status.reset(&unloading_action); 
        FILE_LOG(logINFO) << "Success /unloading/{action_id} GET : " << unloading_status->asJson().toStdString();
        badResponse = false;
        loop->quit();
    });
    // Error call back for Get /unloading/{action_id}
    connect(api.get(), &OAIDefaultApi::unloadingActionIdGetSignalE, this, 
        [&](OAIContainerActionStatus unloading_action , QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure unloading/{action_id} GET :" << error_str.toStdString();
        FILE_LOG(logERROR) << error_code;
        badResponse = true;
        loop->quit();
    });
    // Flag to continue polling until receiving a non error response from server
    do {
        api.get()->unloadingActionIdGet( action_id );
        loop->exec();
        // usleep coversion from seconds to microseconds
        usleep( polling_frequency * 1e6 );

    }
    while( badResponse || unloading_status->getStatus() != QString::fromStdString( "UNLOADED") );
}

int WebServiceClient::pollInspectionAction( QString action_id ) {
    FILE_LOG(logERROR) << "Starting inspection action Polling";
    std::unique_ptr<QEventLoop> loop( new QEventLoop());

    bool badResponse = true;
    // Disconnect all duplicate signals
    disconnect(api.get(), &OAIDefaultApi::inspectionActionIdGetSignal, nullptr, nullptr);
    disconnect(api.get(), &OAIDefaultApi::inspectionActionIdGetSignalE, nullptr, nullptr);
    
    // Call back for GET /inspection/{action_id}
    connect(api.get(), &OAIDefaultApi::inspectionActionIdGetSignal, [&](OAIInspectionStatus inspection) {
        inspection_status.reset( &inspection );
        FILE_LOG(logINFO) << "Success /inspection/{action_id} GET : " << inspection_status->asJson().toStdString() << std::endl;
        badResponse = false;
        loop->quit();
    });
    // Error call back for /inspection/{action_id}
    connect(api.get(), &OAIDefaultApi::inspectionActionIdGetSignalE, 
        [&](OAIInspectionStatus inspection , QNetworkReply::NetworkError error_code, QString error_str) {
        FILE_LOG(logERROR) << "Failure /inspection/{action_id} GET : " << error_str.toStdString();
        FILE_LOG(logERROR) << error_code;
        badResponse = true;
        loop->quit();
    });
    do {
        api->inspectionActionIdGet( action_id );
        loop->exec();

        if (inspection_status->getStatus() == QString::fromStdString( "PASSED")){
            return 0;
        }
        else if (inspection_status->getStatus() == QString::fromStdString( "PROCEED_TO_HOLDING")) {
            return 1;
        }
        // usleep coversion from seconds to microseconds
        usleep( polling_frequency * 1e6 );

    }
    while( badResponse || (inspection_status->getStatus() != QString::fromStdString( "PASSED") &&
         inspection_status->getStatus() != QString::fromStdString( "PROCEED_TO_HOLDING")) );
    return -1;
}

    
