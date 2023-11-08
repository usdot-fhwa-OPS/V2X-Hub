PROJECT(RSUHealthMonitorPlugin VERSION 7.5.1 LANGUAGES CXX)

set(TMX_PLUGIN_NAME "RSU Health Monitor")

find_library(NETSNMPAGENT "netsnmpagent")
find_library(NETSNMPMIBS "netsnmpmibs")
find_library(NETSNMP "netsnmp")
find_library(libasn1c .)

BuildTmxPlugin()

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${NETSNMP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC tmxutils ${NETSNMPAGENT} ${NETSNMPMIBS} ${NETSNMP} ${NETSNMP_LIBRARIES} jsoncpp NemaTode)