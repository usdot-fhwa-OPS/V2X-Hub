PROJECT ( PreemptionPlugin VERSION 7.4.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "Preemption")

FIND_PACKAGE (XercesC REQUIRED)
FIND_PACKAGE (NetSNMP REQUIRED)

find_library(libasn1c .)

BuildTmxPlugin ()

TARGET_INCLUDE_DIRECTORIES ( ${PROJECT_NAME} PUBLIC ${XercesC_INCLUDE_DIRS} ${NETSNMP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} tmxutils ${XercesC_LIBRARY} ${NETSNMP_LIBRARIES})

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(runPreemptionTests_sources
                test/PreemptionTest.cpp 
                test/Main.cpp
)

# Link runPreemptionTests with what we want to test and the GTest and pthread library
add_executable(runPreemptionTests ${runPreemptionTests_sources})

target_link_libraries(runPreemptionTests ${GTEST_LIBRARIES} pthread)
