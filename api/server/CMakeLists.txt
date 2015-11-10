set(SOURCES app.cc
            server.cc
            model.cc
            api.cc
            api_0.cc
            pg.cc
	    graph.cc)

set (SOURCES_P)
foreach (_s ${SOURCES})
    set (SOURCES_P ${SOURCES_P} ${PROJECT_SOURCE_DIR}/api/server/${_s})
endforeach()

add_executable(api_server ${SOURCES_P})

add_dependencies(api_server api_protocol libzmq zmqpp butterfly-genere-version)

set_target_properties(api_server PROPERTIES OUTPUT_NAME butterfly-server)

include_directories(${GLIB2_INCLUDE_DIR} ${GLIB2_INTERNAL_INCLUDE_DIR}
                    ${LIBURCU_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_BINARY_DIR}
                    ${LIBZMQ_INCLUDE_DIR}
                    ${ZMQPP_INCLUDE_DIR}
                    ${PROTOBUF_INCLUDE_DIR}
                    ${LIBNPF_INCLUDE_DIR}
                    ${PG_INCLUDE_DIR}
                    ${DPDK_INCLUDE_DIR}
                    )

target_link_libraries(api_server
                      api_protocol
                      pthread
		      pcap
                      dl
                      ${LIBURCU_LIBRARY}
                      ${PROTOBUF_LIBRARIES}
                      ${GLIB2_LIBRARIES}
                      ${LIBZMQ_LIBRARIES}
                      ${ZMQPP_LIBRARIES}
                      ${PG_LIBRARIES}
                      ${LIBNPF_LIBRARY}
                      ${LIBNPFKERN_LIBRARY}
                      ${DPDK_LIBRARIES}
                      )

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread -static-libgcc -static-libstdc++")

unset (SOURCES_P)
