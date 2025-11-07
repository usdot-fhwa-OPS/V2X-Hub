PROJECT ( PortDrayagePlugin VERSION 7.11.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "PortDrayage")
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wno-unused-variable")

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)


include_directories( "/usr/local/include/pdclient")

BuildTmxPlugin ( )

target_link_libraries(${PROJECT_NAME} PRIVATE pdclient )
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Network ssl crypto ) 
target_link_libraries(${PROJECT_NAME} PUBLIC tmxutils)

