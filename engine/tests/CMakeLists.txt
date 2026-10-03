# All core engine tests are declared here.
# Each test is a separate executable that is linked against the engine library.

orion_add_test(
	NAME orion.test.hash
	SOURCES hash.test.cpp
	DEPENDS orion.engine
)
