cmake_minimum_required (VERSION 2.4)

if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

OPTION(NODEBUG  "Deactivate No debugging option" "OFF")

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fPIC -Wall -Wno-unused-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++14 -Wall -Wno-unused-variable")

if(${NODEBUG} STREQUAL "OFF")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -pg -g3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g3")
else (${NODEBUG} STREQUAL "OFF")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -s -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s -O3")
endif(${NODEBUG} STREQUAL "OFF")


set(PKCS11_PROXY_SRCS gck-rpc-module.c gck-rpc-message.c gck-rpc-util.c egg-buffer.c gck-rpc-tls-psk.c)
set(PKCS11_DAEMON_SRCS egg-buffer.c gck-rpc-daemon-standalone.c gck-rpc-dispatch.c gck-rpc-message.c gck-rpc-util.c syscall-reporter.c syscall-names.h gck-rpc-tls-psk.c)

add_definitions(-Wall)
add_library(pkcs11-proxy SHARED ${PKCS11_PROXY_SRCS})


#add_executable (pkcs11-daemon ${GUI_TYPE} ${PKCS11_DAEMON_SRCS})

set_target_properties(pkcs11-proxy PROPERTIES VERSION 0.1 SOVERSION 0)


target_link_libraries (pkcs11-proxy pthread ssl crypto)
target_link_libraries (pkcs11-daemon dl pthread ssl crypto seccomp)

#install_targets (/lib pkcs11-proxy)
#install_targets (/bin pkcs11-daemon)

install(EXPORT pkcs11-proxyAPIConfig DESTINATION share/pkcs11proxyAPI/cmake)

export(TARGETS pkcs11-proxy FILE pkcs11proxyAPIConfig.cmake)

