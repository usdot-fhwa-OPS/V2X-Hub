PROJECT ( PedestrianPlugin VERSION 7.4.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "Pedestrian")
add_compile_options(-fPIC)
FIND_PACKAGE (XercesC REQUIRED)
FIND_PACKAGE (NetSNMP REQUIRED)
#FIND_PACKAGE (QHttpEngine REQUIRED)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

#set(qserverPedestrian_DIR "/usr/local/share/qserverPedestrian/cmake")
find_package(qserverPedestrian REQUIRED)

include_directories(${Qt5Widgets_INCLUDE_DIRS}) 

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

find_library(libasn1c .)



include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
)


BuildTmxPlugin ()



TARGET_INCLUDE_DIRECTORIES ( ${PROJECT_NAME} PUBLIC ${XercesC_INCLUDE_DIRS} ${NETSNMP_INCLUDE_DIRS} ${Qt5Core_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} PUBLIC tmxutils ${XercesC_LIBRARY} ${NETSNMP_LIBRARIES} ${QHttpEngine_LIBRARY} Qt5Widgets Qt5Core Qt5Network ssl crypto qhttpengine qserverPedestrian) 


link_directories(${CMAKE_PREFIX_PATH}/lib)

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(runPedestrianTests_sources
                test/PedestrianTest.cpp 
                test/Main.cpp
)

# Link runTests with what we want to test and the GTest and pthread library
add_executable(runPedestrianTests ${runPedestrianTests_sources})

target_link_libraries(runPedestrianTests ${GTEST_LIBRARIES} pthread ${QHttpEngine_LIBRARY} Qt5::Widgets Qt5Core Qt5Network ssl crypto qhttpengine)
