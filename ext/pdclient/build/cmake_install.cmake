# Install script for directory: /home/ode/V2X-Hub/ext/pdclient

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/ode/V2X-Hub/ext/pdclient/build/libpdclient.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/pdclient/OAIActionStatusList.h;/usr/local/include/pdclient/OAIContainerActionStatus.h;/usr/local/include/pdclient/OAIContainerRequest.h;/usr/local/include/pdclient/OAIDefaultApi.h;/usr/local/include/pdclient/OAIEnum.h;/usr/local/include/pdclient/OAIHelpers.h;/usr/local/include/pdclient/OAIHttpFileElement.h;/usr/local/include/pdclient/OAIHttpRequest.h;/usr/local/include/pdclient/OAIInspectionRequest.h;/usr/local/include/pdclient/OAIInspectionStatus.h;/usr/local/include/pdclient/OAIInspectionStatusList.h;/usr/local/include/pdclient/OAIObject.h;/usr/local/include/pdclient/OAIServerConfiguration.h;/usr/local/include/pdclient/OAIServerVariable.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/pdclient" TYPE FILE FILES
    "/home/ode/V2X-Hub/ext/pdclient/OAIActionStatusList.h"
    "/home/ode/V2X-Hub/ext/pdclient/OAIContainerActionStatus.h"
    "/home/ode/V2X-Hub/ext/pdclient/OAIContainerRequest.h"
    "/home/ode/V2X-Hub/ext/pdclient/OAIDefaultApi.h"
    "/home/ode/V2X-Hub/ext/pdclient/OAIEnum.h"
    "/home/ode/V2X-Hub/ext/pdclient/OAIHelpers.h"
    "/home/ode/V2X-Hub/ext/pdclient/OAIHttpFileElement.h"
    "/home/ode/V2X-Hub/ext/pdclient/OAIHttpRequest.h"
    "/home/ode/V2X-Hub/ext/pdclient/OAIInspectionRequest.h"
    "/home/ode/V2X-Hub/ext/pdclient/OAIInspectionStatus.h"
    "/home/ode/V2X-Hub/ext/pdclient/OAIInspectionStatusList.h"
    "/home/ode/V2X-Hub/ext/pdclient/OAIObject.h"
    "/home/ode/V2X-Hub/ext/pdclient/OAIServerConfiguration.h"
    "/home/ode/V2X-Hub/ext/pdclient/OAIServerVariable.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/ode/V2X-Hub/ext/pdclient/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
