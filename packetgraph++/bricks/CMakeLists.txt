set(WRAPPER_SOURCE_DIR ${PROJECT_SOURCE_DIR}/packetgraph++/bricks)
set(SOURCES bricks.cpp)

add_library(brickspp OBJECT ${SOURCES})

add_custom_target(brickspp_static ALL
  COMMAND ${CPPCHECK_COMMAND} ${SOURCES}
  WORKING_DIRECTORY ${WRAPPER_SOURCE_DIR})
add_dependencies(brickspp_static brickspp)