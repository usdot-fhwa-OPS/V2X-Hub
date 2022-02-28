#
# Transportation Message eXchange (TMX)
# Makefile configuration script
#
# Author:
#    Gregory M. Baumgardner
# Version:
#    1.0 : Initial version
#

CMAKE_MINIMUM_REQUIRED (VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 14)
# Global values
SET (TMX_APPNAME tmx CACHE INTERNAL "Application name")
SET (TMX_APPDESCR "Transportation Message eXchange (TMX)" CACHE INTERNAL "Application description")

IF (TMX_VERBOSE)
    SET(CMAKE_VERBOSE_MAKEFILE ON CACHE INTERNAL "C Project setting")
    SET(CMAKE_USE_RELATIVE_PATHS ON CACHE INTERNAL "C Project setting")
ENDIF (TMX_VERBOSE)

ENABLE_LANGUAGE ( C )
ENABLE_LANGUAGE ( CXX )

INCLUDE (CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG ("-fPIC" CMAKE_C_FLAG_FPIC_SUPPORTED)

IF (NOT TMX_BIN_DIR)
    SET (TMX_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
ENDIF (NOT TMX_BIN_DIR)

IF (NOT TMX_LIB_DIR)
    SET (TMX_LIB_DIR "${CMAKE_BINARY_DIR}/lib")
ENDIF (NOT TMX_LIB_DIR)

IF (TMX_CROSSCOMPILING)
    MESSAGE (STATUS "Cross-compiling for ${CMAKE_SYSTEM} on ${CMAKE_SYSTEM_PROCESSOR}")
ENDIF (TMX_CROSSCOMPILING)

LINK_DIRECTORIES (${PROJECT_NAME} "${TMX_LIB_DIR}")

#
# Add the generated SAE J2735 library and an interface that contains the build properties
#
ADD_SUBDIRECTORY (Asn_J2735)
ADD_LIBRARY (libasn_j2735 ALIAS ${ASN_J2735_LIBRARIES})
#
# Set up some required platform dependencies, including MySQL
#
SET (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}")
FIND_PACKAGE ( TmxMySQL )

#
# Include the API libraries and interfaces that contain the build properties
#
ADD_SUBDIRECTORY ( TmxApi )

IF (NOT TMX_NO_PLUGINS)
    ADD_SUBDIRECTORY ( Messages )
    ADD_SUBDIRECTORY ( TmxUtils )

    #
    # Now, add the server commands
    #
    ADD_SUBDIRECTORY ( TmxCore )
    ADD_SUBDIRECTORY ( TmxCtl )
    ADD_SUBDIRECTORY ( TmxTools )
    
    # Installs and exports
    EXPORT (TARGETS ${ASN_J2735_LIBRARIES} ${TMXAPI_LIBRARIES} ${TMXUTILS_LIBRARIES} ${TMXCTL_LIBRARIES}
            FILE ${CMAKE_BINARY_DIR}/${TMX_APPNAME}-plugin-config.cmake)
ELSE ()
    EXPORT (TARGETS ${ASN_J2735_LIBRARIES} ${TMXAPI_LIBRARIES}
            FILE ${CMAKE_BINARY_DIR}/${TMX_APPNAME}-plugin-config.cmake)
ENDIF ()

INSTALL (EXPORT ${TMX_APPNAME} DESTINATION share/${TMX_APPNAME}
			FILE ${TMX_APPNAME}-plugin-config.cmake)
