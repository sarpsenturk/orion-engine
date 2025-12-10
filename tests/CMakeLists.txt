# Download GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip # 1.17.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

function(orion_add_test name src)
	add_executable(${name} ${src})
	target_link_libraries(${name} orion GTest::gtest_main)
	include(GoogleTest)
	gtest_discover_tests(${name})
endfunction()

orion_add_test(vector vector.cpp)
orion_add_test(matrix matrix.cpp)
orion_add_test(transform transform.cpp)