PROJECT ( CARMACloudPlugin VERSION 3.0.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "CARMACloud")
add_compile_options(-fPIC)
FIND_PACKAGE (XercesC REQUIRED)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(v2xhubWebAPI REQUIRED)
find_package(CURL REQUIRED)

include_directories(${Qt5Widgets_INCLUDE_DIRS}) 

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

find_library(libasn1c .)

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
)


BuildTmxPlugin ()



TARGET_INCLUDE_DIRECTORIES ( ${PROJECT_NAME} PUBLIC ${XercesC_INCLUDE_DIRS} ${NETSNMP_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} PUBLIC tmxutils ${XercesC_LIBRARY} ${NETSNMP_LIBRARIES} ${QHttpEngine_LIBRARY}  ${Boost_SYSTEM_LIBRARY} curl cpprest Qt5Widgets Qt5Core Qt5Network ssl crypto qhttpengine v2xhubWebAPI) 


link_directories(${CMAKE_PREFIX_PATH}/lib)