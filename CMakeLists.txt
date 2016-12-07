cmake_minimum_required(VERSION 2.6)
include(ExternalProject)
project(BUTTERFLY)

# 3rdparties directories
set(3RDPARTY_DIR ${PROJECT_SOURCE_DIR}/3rdparty)

set(3RDPARTY_INSTALL_DIR ${PROJECT_BINARY_DIR}/3rdparty-build)
set(DPDK_INSTALL_DIR ${3RDPARTY_INSTALL_DIR})
set(LIBZMQ_INSTALL_DIR ${3RDPARTY_INSTALL_DIR}/libzmq)
set(ZMQPP_INSTALL_DIR ${3RDPARTY_INSTALL_DIR}/zmqpp)
set(PROTOBUF_INSTALL_DIR ${3RDPARTY_INSTALL_DIR}/protobuf)
set(PG_INSTALL_DIR ${3RDPARTY_INSTALL_DIR}/packetgraph)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

find_package(GLIB2 REQUIRED)

set(PROTOBUF_LIBRARIES
	${PROTOBUF_INSTALL_DIR}/src/.libs/libprotobuf-lite.a
	${PROTOBUF_INSTALL_DIR}/src/.libs/libprotobuf.a)

set(PROTOBUF_INCLUDE_DIR
	${3RDPARTY_DIR}/protobuf/src
	${PROJECT_BINARY_DIR})

set(PROTOBUF_PROTOC_EXECUTABLE ${PROTOBUF_INSTALL_DIR}/src/protoc)
set(LIBZMQ_LIBRARIES ${LIBZMQ_INSTALL_DIR}/lib/libzmq-static.a)
set(LIBZMQ_INCLUDE_DIR ${3RDPARTY_DIR}/libzmq/include)
set(ZMQPP_LIBRARIES ${ZMQPP_INSTALL_DIR}/libzmqpp-static.a)
set(ZMQPP_INCLUDE_DIR ${3RDPARTY_DIR}/zmqpp/src)
set(PG_LIBRARIES
    -L${DPDK_INSTALL_DIR}/build/lib -Wl,--start-group
    -Wl,--whole-archive,${DPDK_INSTALL_DIR}/build/lib/libdpdk.a,--no-whole-archive
    ${PG_INSTALL_DIR}/.libs/libbpfjit.a
    ${PG_INSTALL_DIR}/.libs/libcdb.a
    ${PG_INSTALL_DIR}/.libs/libnpf.a
    ${PG_INSTALL_DIR}/.libs/libnpfkern.a
    ${PG_INSTALL_DIR}/.libs/libprop.a
    ${PG_INSTALL_DIR}/.libs/libqsbr.a
    ${PG_INSTALL_DIR}/.libs/libsljit.a
    ${PG_INSTALL_DIR}/.libs/libpacketgraph.a -Wl,--end-group
    -lz)
set(PG_INCLUDE_DIR ${3RDPARTY_DIR}/packetgraph/include)

set(CMAKE_C_FLAGS "-g -O3 -march=core-avx-i -mtune=core-avx-i -fmessage-length=0  -Werror -Wall -Wextra -Wwrite-strings -Winit-self -Wcast-align -Wpointer-arith -Wstrict-aliasing -Wformat=2 -Wmissing-declarations -Wmissing-include-dirs -Wno-unused-parameter -Wuninitialized -Wold-style-definition -Wstrict-prototypes -Wmissing-prototypes -L${DPDK_INSTALL_DIR}/build/lib -Wl,--whole-archive,${DPDK_INSTALL_DIR}/build/lib/libdpdk.a,--no-whole-archive")

# uninstall target
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
      COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

add_custom_target(butterfly-genere-version
  COMMAND ${PROJECT_SOURCE_DIR}/scripts/genHdrVersion.sh ${PROJECT_BINARY_DIR}/api/version.h
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )

add_subdirectory(3rdparty)
add_subdirectory(api)

# Add custom targets for tests

add_custom_target(tests-all
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/tests_all.sh ${PROJECT_BINARY_DIR} ${VERBOSE}
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_custom_target(style
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/tests_api_style.sh
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_custom_target(tests-api
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/tests_api.sh ${PROJECT_BINARY_DIR} ${VERBOSE}
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_custom_target(tests-scenario
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/tests_scenario.sh ${PROJECT_BINARY_DIR} ${VERBOSE}
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_custom_target(package-rpm
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/build_package.sh ${PROJECT_BINARY_DIR} rpm
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_custom_target(package-deb
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/build_package.sh ${PROJECT_BINARY_DIR} deb
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_custom_target(package-fat-rpm
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/build_fat_package.sh ${PROJECT_BINARY_DIR} rpm
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_custom_target(package-fat-deb
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/build_fat_package.sh ${PROJECT_BINARY_DIR} deb
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_custom_target(package-fat
		  COMMAND rm -rvf ${PROJECT_BINARY_DIR}/fat-butterfly*
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/dep-gets.sh ${PROJECT_BINARY_DIR}/api/server/butterflyd ${PROJECT_BINARY_DIR}/fat-butterflyd
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/dep-gets.sh ${PROJECT_BINARY_DIR}/api/client/butterfly ${PROJECT_BINARY_DIR}/fat-butterfly
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
