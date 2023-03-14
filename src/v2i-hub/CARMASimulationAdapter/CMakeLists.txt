PROJECT ( CARMASimulationAdapter VERSION 7.4.0 LANGUAGES CXX )
FIND_PACKAGE( carma-clock )
BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils ::carma-clock rdkafka++ jsoncpp)
