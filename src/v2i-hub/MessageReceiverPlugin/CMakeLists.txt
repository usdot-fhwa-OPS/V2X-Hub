PROJECT ( MessageReceiverPlugin VERSION 7.11.1 LANGUAGES CXX )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set (TMX_PLUGIN_NAME "MessageReceiver")
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(stol-1609dot2-2022 REQUIRED)

BuildTmxPlugin ()
target_include_directories( ${PROJECT_NAME} PUBLIC /opt/carma/include/stol-1609dot2-2022)
target_link_libraries( ${PROJECT_NAME} tmxutils ::stol-1609dot2-2022) 
