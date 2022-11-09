PROJECT ( SPaTLoggerPlugin VERSION 7.3.1 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "SPaTLoggerPlugin")

BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils)
