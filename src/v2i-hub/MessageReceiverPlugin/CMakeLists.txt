PROJECT ( MessageReceiverPlugin VERSION 7.4.0 LANGUAGES CXX )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET (TMX_PLUGIN_NAME "MessageReceiver")
SET (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
#SET(CMAKE_BOTAN_SOURCE_DIR {CMAKE_SOURCE_DIR}/../../../ext/botan)
#include_directories(${CMAKE_BOTAN_SOURCE_DIR}/build/include)

#add_library( libbotan-2 SHARED IMPORTED )
#set_target_properties( libbotan-2 PROPERTIES IMPORTED_LOCATION /usr/local/lib/libbotan-2.so )


BuildTmxPlugin ()

TARGET_INCLUDE_DIRECTORIES ( ${PROJECT_NAME} PUBLIC )
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} tmxutils ) #libbotan-2)
