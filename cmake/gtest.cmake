# Use FetchContent to download and add googletest
set(
	ORION_GOOGLETEST_COMMIT_HASH
	"063de7e9578f82b369302001269680b4b1553359"
	CACHE STRING "The commit hash of the googletest version to use"
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL "https://github.com/google/googletest/archive/${ORION_GOOGLETEST_COMMIT_HASH}.zip"
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# Create a test target with the given name, sources, and dependencies
function(orion_add_test)
	# Arguments:
	#   NAME: The name of the test target
	#   SOURCES: The source files for the test target
	#   DEPENDENCIES: The dependencies for the test target (optional)
	set(options)
	set(oneValueArgs NAME)
	set(multiValueArgs SOURCES DEPENDS)
	cmake_parse_arguments(TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	if (NOT TEST_NAME)
		message(FATAL_ERROR "orion_add_test: NAME argument is required")
	endif()
	if (NOT TEST_SOURCES)
		message(FATAL_ERROR "orion_add_test: SOURCES argument is required")
	endif()
	add_executable(${TEST_NAME} ${TEST_SOURCES})
	target_link_libraries(${TEST_NAME} PRIVATE gtest gtest_main ${TEST_DEPENDS})
	gtest_discover_tests(${TEST_NAME})
endfunction()
