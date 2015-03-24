set(UTILS_WRAPPER_SOURCE_DIR ${PROJECT_SOURCE_DIR}/packetgraph++/utils)
set(SOURCES lifecycle.cpp)

add_library(utilspp OBJECT ${SOURCES})

add_custom_target(utilspp_static ALL
  COMMAND ${CPPCHECK_COMMAND} ${SOURCES}
  WORKING_DIRECTORY ${UTILS_WRAPPER_SOURCE_DIR})
add_dependencies(utilspp_static utilspp)