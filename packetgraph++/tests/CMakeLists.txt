set(TESTS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/packetgraph++/tests)
set(SOURCES tests.cpp)

add_executable(testspp ${SOURCES}
		$<TARGET_OBJECTS:packets>
		$<TARGET_OBJECTS:utils>
		$<TARGET_OBJECTS:bricks>
		$<TARGET_OBJECTS:bricks_debug>
		$<TARGET_OBJECTS:utilspp>)
target_link_libraries (testspp ${GLIB2_LIBRARIES} ${TARGET_LIBRARIES})

add_custom_target(testspp_static ALL
	COMMAND ${CPPCHECK_COMMAND} ${SOURCES}
	WORKING_DIRECTORY ${TESTS_SOURCE_DIR})
add_dependencies(testspp_static testspp)
