PROJECT ( PortDrayagePlugin VERSION 5.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "PortDrayage")

find_package(Qt5Network REQUIRED)
find_package(Qt5Core REQUIRED)

target_include_directories( ${PROJECT_NUAME} PUBLIC "/usr/local/include" "/usr/local/lib")



message(CMAKE MODULE : ${CMAKE_MODULE_PATH} )


BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} PRIVATE libpdclient Qt5::Core Qt5::Network )
TARGET_LINK_LIBRARIES (${PROJECT_NAME} PUBLIC tmxutils)