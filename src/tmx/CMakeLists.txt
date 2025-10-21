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
set(CMAKE_CXX_STANDARD 17)
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
# New Option to set to just SAEJ2735 Version revision number. Defaults to 2016 or call cmake -Bbuild -DSAEJ2735_SPEC_VERSION=<rnumber>
LIST(APPEND CMAKE_PREFIX_PATH "/opt/carma/lib")
LIST(APPEND CMAKE_MODULE_PATH "/opt/carma/cmake")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
SET(J2735_LIBRARY_NAME "")
SET(SAEJ2735_SPEC_VERSION "2024" CACHE STRING "Option to set version of the SAE J2735 specification to link as ASN_J2735_LIBRARIES.")
IF( SAEJ2735_SPEC_VERSION EQUAL "2016")
    SET(J2735_LIBRARY_NAME "::stol-j2735-201603-carma")
    # TODO Update stol-j2735 2016 version to remove 03 suffix which will allow for
    # removal of SAJ2735_SPEC_FULL_VERSION variable
    SET(SAEJ2735_SPEC_FULL_VERSION "201603")

    FIND_PACKAGE(stol-j2735-201603-carma REQUIRED)
ELSEIF( SAEJ2735_SPEC_VERSION EQUAL "2020")
    SET(J2735_LIBRARY_NAME "::stol-j2735-2020-carma")
    SET(SAEJ2735_SPEC_FULL_VERSION "2020")

    FIND_PACKAGE(stol-j2735-2020-carma REQUIRED)
ELSEIF( SAEJ2735_SPEC_VERSION EQUAL "2024")
    SET(J2735_LIBRARY_NAME "::stol-j2735-2024-carma")
    SET(SAEJ2735_SPEC_FULL_VERSION "2024")

    FIND_PACKAGE(stol-j2735-2024-carma REQUIRED)
ELSE()
    MESSAGE(FATAL_ERROR "Invalid SAEJ2735_SPEC_VERSION: ${SAEJ2735_SPEC_VERSION}")
ENDIF()

#
# Set up some required platform dependencies, including MySQL
#
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
    EXPORT (TARGETS ${TMXAPI_LIBRARIES} ${TMXUTILS_LIBRARIES} ${TMXCTL_LIBRARIES}
            FILE ${CMAKE_BINARY_DIR}/${TMX_APPNAME}-plugin-config.cmake)
ELSE ()
    EXPORT (TARGETS ${TMXAPI_LIBRARIES}
            FILE ${CMAKE_BINARY_DIR}/${TMX_APPNAME}-plugin-config.cmake)
ENDIF ()

INSTALL (EXPORT ${TMX_APPNAME} DESTINATION share/${TMX_APPNAME}
			FILE ${TMX_APPNAME}-plugin-config.cmake)

