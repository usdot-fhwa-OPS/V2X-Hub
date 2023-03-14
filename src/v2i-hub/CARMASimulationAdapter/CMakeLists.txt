PROJECT ( CARMASimulationAdapter VERSION 7.4.0 LANGUAGES CXX )

BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils kafkaclient rdkafka++ jsoncpp)