PROJECT ( IntersectionValidationPlugin VERSION 7.11.1 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "Intersection Validation")

FIND_PACKAGE (carma-clock)

BuildTmxPlugin ()

TARGET_INCLUDE_DIRECTORIES ( ${PROJECT_NAME} PUBLIC  )
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} tmxutils ::carma-clock )