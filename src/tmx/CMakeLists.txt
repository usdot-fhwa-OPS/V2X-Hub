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

