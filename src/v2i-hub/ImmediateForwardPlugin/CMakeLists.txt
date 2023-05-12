PROJECT ( ImmediateForwardPlugin VERSION 7.5.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "Immediate Forward")

FIND_PACKAGE (XercesC REQUIRED)
find_package(CURL REQUIRED)

#SET(CMAKE_BOTAN_SOURCE_DIR {CMAKE_SOURCE_DIR}/../../../ext/botan)
#include_directories(${CMAKE_BOTAN_SOURCE_DIR}/build/include)
#add_library( libbotan-2 SHARED IMPORTED )
#set_target_properties( libbotan-2 PROPERTIES IMPORTED_LOCATION /usr/local/lib/libbotan-2.so )


BuildTmxPlugin ()



TARGET_INCLUDE_DIRECTORIES ( ${PROJECT_NAME} PUBLIC ${XercesC_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} tmxutils ${XercesC_LIBRARY} ) #libbotan-2 )
