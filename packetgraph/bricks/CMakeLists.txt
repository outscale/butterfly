set(SOURCES bricks.c nop-brick.c collect-brick.c switch-brick.c diode-brick.c
	    vhost-user-brick.c)
set(SOURCES_DEBUG packetsgen-brick.c)
set(BRICKS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/packetgraph/bricks)

add_library(bricks OBJECT ${SOURCES})
add_library(bricks_debug OBJECT ${SOURCES_DEBUG})
add_dependencies(bricks 3rdparty)

add_custom_target(bricks_static ALL
	COMMAND ${CPPCHECK_COMMAND} ${SOURCES} ${SOURCES_DEBUG}
	WORKING_DIRECTORY ${BRICKS_SOURCE_DIR})
add_dependencies(bricks_static bricks)
