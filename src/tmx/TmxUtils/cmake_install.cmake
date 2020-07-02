# Install script for directory: /home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils

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

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibtmxutilsx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/lib/libtmxutils.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibtmxutilsx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/Base64.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/BsmConverter.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/Clock.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/Conversions.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/DigitalDevice.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/FrequencyThrottle.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/GeoDisplay.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/GeoVector.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/ITISPhrase.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/LockFreeThread.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/Logger.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/ManualResetEvent.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/MapSupport.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/ParsedMap.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/PluginClient.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/PluginDataMonitor.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/PluginException.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/PluginExec.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/PluginExtender.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/PluginKeepAlive.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/PluginLog.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/PluginUpgrader.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/PluginUtil.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/Region.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/SignalException.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/StringParser.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/System.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/SystemContextThread.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/ThreadGroup.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/ThreadTimer.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/ThreadWorker.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/TmxLog.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/TmxMessageManager.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/TransitVehicle.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/UdpClient.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/UdpServer.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/Uuid.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/WGS84Point.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/WGS84Polygon.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/database/DbConnection.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/database/DbConnectionPool.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/database/SystemContext.h"
    "/home/anjanrayamajhi/Projects/V2X-CARMACloud/V2X-Hub/src/tmx/TmxUtils/src/version.h"
    )
endif()

