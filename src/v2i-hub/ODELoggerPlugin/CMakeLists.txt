PROJECT ( ODELoggerPlugin VERSION 7.4.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "ODELoggerPlugin")

BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils rdkafka++)
