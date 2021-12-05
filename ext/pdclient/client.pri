QT += network

HEADERS += \
# Models
    $${PWD}/OAIActionStatusList.h \
    $${PWD}/OAIContainerActionStatus.h \
    $${PWD}/OAIContainerRequest.h \
    $${PWD}/OAIInspectionRequest.h \
    $${PWD}/OAIInspectionStatus.h \
    $${PWD}/OAIInspectionStatusList.h \
# APIs
    $${PWD}/OAIDefaultApi.h \
# Others
    $${PWD}/OAIHelpers.h \
    $${PWD}/OAIHttpRequest.h \
    $${PWD}/OAIObject.h \
    $${PWD}/OAIEnum.h \
    $${PWD}/OAIHttpFileElement.h \
    $${PWD}/OAIServerConfiguration.h \
    $${PWD}/OAIServerVariable.h

SOURCES += \
# Models
    $${PWD}/OAIActionStatusList.cpp \
    $${PWD}/OAIContainerActionStatus.cpp \
    $${PWD}/OAIContainerRequest.cpp \
    $${PWD}/OAIInspectionRequest.cpp \
    $${PWD}/OAIInspectionStatus.cpp \
    $${PWD}/OAIInspectionStatusList.cpp \
# APIs
    $${PWD}/OAIDefaultApi.cpp \
# Others
    $${PWD}/OAIHelpers.cpp \
    $${PWD}/OAIHttpRequest.cpp \
    $${PWD}/OAIHttpFileElement.cpp
