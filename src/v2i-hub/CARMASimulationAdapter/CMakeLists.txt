PROJECT ( CARMASimulationAdapter VERSION 7.4.0 LANGUAGES CXX )
set(CMAKE_CXX_STANDARD 17)

FIND_PACKAGE( carma-clock )
BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils ::carma-clock rdkafka++ jsoncpp)
