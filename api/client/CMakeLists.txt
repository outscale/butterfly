set(SOURCES client.cc nic.cc request.cc sg.cc status.cc shutdown.cc ../common/crypto.cc dump.cc)
set (SOURCES_P)
foreach (_s ${SOURCES})
    set (SOURCES_P ${SOURCES_P} ${PROJECT_SOURCE_DIR}/api/client/${_s})
endforeach()

add_executable(api_client ${SOURCES_P})

add_dependencies(api_client api_protocol libzmq zmqpp butterfly-genere-version)

install(TARGETS api_client DESTINATION bin)

set_target_properties(api_client PROPERTIES OUTPUT_NAME butterfly)

include_directories(${GLIB2_INCLUDE_DIR} ${GLIB2_INTERNAL_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_BINARY_DIR}
                    ${PROTOBUF_INCLUDE_DIR}
                    ${LIBZMQ_INCLUDE_DIR}
                    ${ZMQPP_INCLUDE_DIR})

target_link_libraries(api_client
                      api_protocol
                      pthread
                      crypto
                      ${PROTOBUF_LIBRARIES}
                      ${LIBZEROMQPP_LIBRARY}
                      ${GLIB2_LIBRARIES}
                      ${LIBZMQ_LIBRARIES}
                      ${ZMQPP_LIBRARIES})

if(COVERAGE)
   set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -Wall -std=c++11 -static-libstdc++")
else()
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -Wall -static-libgcc -static-libstdc++")
endif()
unset (SOURCES_P)
