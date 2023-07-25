PROJECT ( ODEForwardPlugin VERSION 7.5.1 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "ODEForwardPlugin")

BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils rdkafka++)
