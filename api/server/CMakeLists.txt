set(SOURCES app.cc
            server.cc
            model.cc
            api.cc
            api_0.cc)

set (SOURCES_P)
foreach (_s ${SOURCES})
    set (SOURCES_P ${SOURCES_P} ${PROJECT_SOURCE_DIR}/api/server/${_s})
endforeach()

add_executable(api_server ${SOURCES_P})

add_dependencies(api_server api_protocol libzmq zmqpp)

set_target_properties(api_server PROPERTIES OUTPUT_NAME butterfly-server)

include_directories(${GLIB2_INCLUDE_DIR} ${GLIB2_INTERNAL_INCLUDE_DIR}
                    ${LIBURCU_INCLUDE_DIR}
		    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_BINARY_DIR}
                    ${LIBZMQ_INCLUDE_DIR}
                    ${ZMQPP_INCLUDE_DIR}
                    ${PROTOBUF_INCLUDE_DIR})

target_link_libraries(api_server
                      api_protocol
                      pthread
                      ${PROTOBUF_LIBRARIES}
                      ${GLIB2_LIBRARIES}
                      ${LIBZMQ_LIBRARIES}
                      ${ZMQPP_LIBRARIES}
		      ${DPDK_INSTALL_DIR}/lib/libethdev.a
		      ${DPDK_INSTALL_DIR}/lib/librte_hash.a
		      ${DPDK_INSTALL_DIR}/lib/librte_mbuf.a
		      ${DPDK_INSTALL_DIR}/lib/librte_mempool.a
		      ${DPDK_INSTALL_DIR}/lib/librte_table.a
		      ${DPDK_INSTALL_DIR}/lib/librte_ring.a
		      ${DPDK_INSTALL_DIR}/lib/librte_eal.a
		      ${DPDK_INSTALL_DIR}/lib/librte_malloc.a
		      ${DPDK_INSTALL_DIR}/lib/librte_vhost.a
		      ${DPDK_INSTALL_DIR}/lib/librte_pmd_pcap.a
		      ${DPDK_INSTALL_DIR}/lib/librte_pmd_ring.a
		      ${DPDK_INSTALL_DIR}/lib/librte_kvargs.a
		      ${DPDK_INSTALL_DIR}/lib/librte_pmd_e1000.a
		      ${DPDK_INSTALL_DIR}/lib/librte_pmd_ixgbe.a
		      )

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

unset (SOURCES_P)
