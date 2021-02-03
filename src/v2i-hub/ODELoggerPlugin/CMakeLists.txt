PROJECT ( ODELoggerPlugin VERSION 5.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "ODELoggerPlugin")


#include( FetchContent )
#FetchContent_Declare( date_src
#	GIT_REPOSITORY https://github.com/HowardHinnant/date.git
#	GIT_TAG        v3.0.0  # adjust tag/branch/commit as needed
#	)
#FetchContent_MakeAvailable(date_src)
  
#target_link_libraries (my_target PRIVATE date::date)


BuildTmxPlugin ( )

TARGET_LINK_LIBRARIES (${PROJECT_NAME} tmxutils rdkafka++)
