PROJECT ( CARMAStreetsPlugin VERSION 5.0 LANGUAGES CXX )

BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils rdkafka++)