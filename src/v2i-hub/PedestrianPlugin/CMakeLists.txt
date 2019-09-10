PROJECT ( PedestrianPlugin VERSION 3.0.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "Pedestrian")
add_compile_options(-fPIC)
FIND_PACKAGE (XercesC REQUIRED)
FIND_PACKAGE (NetSNMP REQUIRED)
FIND_PACKAGE (QHttpEngine REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

include_directories(${Qt5Widgets_INCLUDE_DIRS}) 

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/external)

ExternalProject_Add(QHTTPENGINE
    GIT_REPOSITORY https://github.com/etherealjoy/qhttpengine.git
    CMAKE_ARGS  -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
                -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

find_library(libasn1c .)

BuildTmxPlugin ()

add_dependencies(${PROJECT_NAME} QHttpEngine)

file(GLOB SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/handlers/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/requests/*.cpp
)

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/handlers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/requests
)


TARGET_INCLUDE_DIRECTORIES ( ${PROJECT_NAME} PUBLIC ${XercesC_INCLUDE_DIRS} ${NETSNMP_INCLUDE_DIRS} ${QHttpEngine_INCLUDE_DIRS} ${SRCS})


TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} PUBLIC tmxutils ${XercesC_LIBRARY} ${NETSNMP_LIBRARIES} ${QHttpEngine_LIBRARY} Qt5::Widgets Qt5Core Qt5Network ssl crypto qhttpengineb) 


#link_directories(${CMAKE_PREFIX_PATH}/lib)

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(runTests_sources
                test/PedestrianTest.cpp 
                test/Main.cpp
)

# Link runTests with what we want to test and the GTest and pthread library
add_executable(runPedestrianTest ${runTests_sources})

target_link_libraries(runPedestrianTest ${GTEST_LIBRARIES} pthread ${QHttpEngine_LIBRARY} Qt5::Widgets Qt5Core Qt5Network ssl crypto qhttpengine)