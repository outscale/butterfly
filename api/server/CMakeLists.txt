set(SOURCES app.cc
            server.cc
            model.cc
            api.cc
            api_0.cc
            graph.cc
            encrypted.cc
            ../common/crypto.cc)

set (SOURCES_P)
foreach (_s ${SOURCES})
    set (SOURCES_P ${SOURCES_P} ${PROJECT_SOURCE_DIR}/api/server/${_s})
endforeach()

add_executable(api_server ${SOURCES_P})

add_dependencies(api_server api_protocol libzmq zmqpp butterfly-genere-version)

set_target_properties(api_server PROPERTIES OUTPUT_NAME butterflyd)

install(TARGETS api_server DESTINATION bin)
install(FILES butterflyd.ini DESTINATION etc/butterfly/ COMPONENT config)

include_directories(${GLIB2_INCLUDE_DIR} ${GLIB2_INTERNAL_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_BINARY_DIR}
                    ${LIBZMQ_INCLUDE_DIR}
                    ${ZMQPP_INCLUDE_DIR}
                    ${PROTOBUF_INCLUDE_DIR}
                    ${PG_INCLUDE_DIR}
                    )

target_link_libraries(api_server
                      api_protocol
                      pthread
                      pcap
                      dl
                      crypto
                      ${PROTOBUF_LIBRARIES}
                      ${GLIB2_LIBRARIES}
                      ${LIBZMQ_LIBRARIES}
                      ${ZMQPP_LIBRARIES}
                      ${PG_LIBRARIES}
                      )

if(COVERAGE)
   set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -Wall -std=c++11")
   set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS} -static-libstdc++ -pthread")
else()
   set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -Wall -g")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread -static-libgcc -static-libstdc++")
endif()
unset (SOURCES_P)
