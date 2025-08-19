project( JSONMessageLoggerPlugin VERSION 7.6.0 LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 17)

set (TMX_PLUGIN_NAME "JSON Message Logger")

find_package(carma-clock REQUIRED)
BuildTmxPlugin()

target_link_libraries(JSONMessageLoggerPlugin PUBLIC Boost::log
    tmxutils 
    ::carma-clock )

