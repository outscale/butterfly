set(SOURCES firewall.c)

add_executable(firewall ${SOURCES}
		$<TARGET_OBJECTS:packets>
		$<TARGET_OBJECTS:utils>
		$<TARGET_OBJECTS:bricks>
		$<TARGET_OBJECTS:bricks_stubs>)

target_link_libraries (firewall ${TARGET_LIBRARIES})
