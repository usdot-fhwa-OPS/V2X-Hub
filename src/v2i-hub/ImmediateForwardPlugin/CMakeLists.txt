project( ImmediateForwardPlugin VERSION 7.6.0 LANGUAGES CXX )

set(TMX_PLUGIN_NAME "Immediate Forward")

find_package(CURL REQUIRED)

BuildTmxPlugin ()

target_include_directories(${PROJECT_NAME} PUBLIC ${CURL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} tmxutils )
