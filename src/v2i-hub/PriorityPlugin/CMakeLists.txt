PROJECT ( PriorityPlugin VERSION 7.11.1 LANGUAGES CXX )
SET (CMAKE_CXX_STANDARD 17)
SET (TMX_PLUGIN_NAME "Priority")

FIND_PACKAGE (XercesC REQUIRED)
FIND_PACKAGE (NetSNMP REQUIRED)

BuildTmxPlugin ()

TARGET_INCLUDE_DIRECTORIES ( ${PROJECT_NAME} PUBLIC ${XercesC_INCLUDE_DIRS} ${NETSNMP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} tmxutils ${XercesC_LIBRARY} ${NETSNMP_LIBRARIES} pthread)

#############
## Testing ##
#############
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(runPriorityTests_sources test/TestPriority.cpp)
add_executable(runPriorityTests ${runPriorityTests_sources})

target_include_directories(runPriorityTests PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${XercesC_INCLUDE_DIRS}
        ${NETSNMP_INCLUDE_DIRS}
)

target_link_libraries(runPriorityTests
        tmxutils
        ${XercesC_LIBRARY}
        ${NETSNMP_LIBRARIES}
        gtest gtest_main pthread
)
