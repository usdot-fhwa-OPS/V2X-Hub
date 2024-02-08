PROJECT ( ExamplePlugin VERSION 3.0.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "PhantomTraffic")

BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils)
