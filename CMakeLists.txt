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
find_package(LibURCU REQUIRED)
find_package(NPF REQUIRED)
find_package(NPFKERN REQUIRED)

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
set(PG_LIBRARIES ${PG_INSTALL_DIR}/firewall/libpacketgraph-firewall.a
                 ${PG_INSTALL_DIR}/nic/libpacketgraph-nic.a
                 ${PG_INSTALL_DIR}/vhost/libpacketgraph-vhost.a
                 ${PG_INSTALL_DIR}/core/libpacketgraph-core.a
                 ${PG_INSTALL_DIR}/print/libpacketgraph-print.a
                 ${PG_INSTALL_DIR}/vtep/libpacketgraph-vtep.a
                 ${PG_INSTALL_DIR}/diode/libpacketgraph-diode.a
                 ${PG_INSTALL_DIR}/antispoof/libpacketgraph-antispoof.a
                 ${PG_INSTALL_DIR}/switch/libpacketgraph-switch.a)
set(PG_INCLUDE_DIR ${3RDPARTY_DIR}/packetgraph/core
                   ${3RDPARTY_DIR}/packetgraph/firewall
                   ${3RDPARTY_DIR}/packetgraph/nic
                   ${3RDPARTY_DIR}/packetgraph/vhost
                   ${3RDPARTY_DIR}/packetgraph/hub
                   ${3RDPARTY_DIR}/packetgraph/print
                   ${3RDPARTY_DIR}/packetgraph/vtep
                   ${3RDPARTY_DIR}/packetgraph/diode
                   ${3RDPARTY_DIR}/packetgraph/antispoof
                   ${3RDPARTY_DIR}/packetgraph/switch)
set(DPDK_LIBRARIES ${DPDK_INSTALL_DIR}/lib/libethdev.a
                   ${DPDK_INSTALL_DIR}/lib/librte_hash.a
                   ${DPDK_INSTALL_DIR}/lib/librte_mbuf.a
                   ${DPDK_INSTALL_DIR}/lib/librte_mempool.a
                   ${DPDK_INSTALL_DIR}/lib/librte_table.a
                   ${DPDK_INSTALL_DIR}/lib/librte_ring.a
                   ${DPDK_INSTALL_DIR}/lib/librte_eal.a
                   ${DPDK_INSTALL_DIR}/lib/librte_vhost.a
                   ${DPDK_INSTALL_DIR}/lib/librte_pmd_pcap.a
                   ${DPDK_INSTALL_DIR}/lib/librte_pmd_ring.a
                   ${DPDK_INSTALL_DIR}/lib/librte_kvargs.a
                   ${DPDK_INSTALL_DIR}/lib/librte_pmd_e1000.a
                   ${DPDK_INSTALL_DIR}/lib/librte_pmd_ixgbe.a)
set(DPDK_INCLUDE_DIR ${DPDK_INSTALL_DIR}/include)

set(CMAKE_C_FLAGS "-g -O3 -march=core-avx-i -mtune=core-avx-i -fmessage-length=0  -Werror -Wall -Wextra -Wwrite-strings -Winit-self -Wcast-align -Wpointer-arith -Wstrict-aliasing -Wformat=2 -Wmissing-declarations -Wmissing-include-dirs -Wno-unused-parameter -Wuninitialized -Wold-style-definition -Wstrict-prototypes -Wmissing-prototypes")

add_custom_target(butterfly-genere-version
  COMMAND ${PROJECT_SOURCE_DIR}/scripts/genHdrVersion.sh ${PROJECT_BINARY_DIR}/version.h
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )

add_subdirectory(3rdparty)
add_subdirectory(api)

# Add custom targets for tests

add_custom_target(tests-all
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/tests_all.sh ${PROJECT_SOURCE_DIR}
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_custom_target(style
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/tests_api_style.sh ${PROJECT_SOURCE_DIR}
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_custom_target(tests-api
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/tests_api.sh ${PROJECT_SOURCE_DIR}
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

add_custom_target(tests-scenario
                  COMMAND ${PROJECT_SOURCE_DIR}/scripts/tests_scenario.sh ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
                  WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
