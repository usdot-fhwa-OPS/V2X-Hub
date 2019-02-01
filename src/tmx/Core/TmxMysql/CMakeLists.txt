set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" 
    ${CMAKE_SOURCE_DIR}/../../../../TMX-OAM/Build/Modules
    ${CMAKE_SOURCE_DIR}/../../../TMX-OAM/Build/Modules
    $ENV{HOME}/TMX-OAM/Build/Modules
    $ENV{TMX_OAM_HOME}/Build/Modules
)
cmake_minimum_required(VERSION 2.8.12)

project( TmxMysql NONE )

set(CMAKE_INSTALL_PREFIX /var/tmp)

set(CPACK_PACKAGE_NAME "tmx-mysql")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Battelle TMX MySQL database")
set(CPACK_DEBIAN_PACKAGE_DEPENDS debconf mysql-server)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA 
    ${CMAKE_CURRENT_SOURCE_DIR}/debian/postinst
    ${CMAKE_CURRENT_SOURCE_DIR}/debian/prerm
    ${CMAKE_CURRENT_SOURCE_DIR}/debian/templates)
include(TMX_Packaging)
include(CPack)

if(NOT MIPS_BUILD)
    install (FILES ${CMAKE_SOURCE_DIR}/localhost.sql
              DESTINATION tmx)
endif(NOT MIPS_BUILD)
