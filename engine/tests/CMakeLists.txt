# All core engine tests are declared here.
# Each test is a separate executable that is linked against the engine library.

orion_add_test(
	NAME orion.test.slot_map
	SOURCES "slot_map.test.cpp"
	DEPENDS orion.engine
)

orion_add_test(
    NAME orion.test.inplace_function
    SOURCES "inplace_function.test.cpp"
    DEPENDS orion.engine
)
