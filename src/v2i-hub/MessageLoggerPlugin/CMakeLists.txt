PROJECT ( MessageLoggerPlugin VERSION 7.5.1 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "MessageLoggerPlugin")

BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils)
