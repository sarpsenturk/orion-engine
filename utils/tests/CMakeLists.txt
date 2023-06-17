find_package(GTest REQUIRED)

function(add_orion_utils_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} orion::utils GTest::gtest_main)
    gtest_discover_tests(${name})
endfunction()

add_orion_utils_test(bitflag)
add_orion_utils_test(static_vector)
add_orion_utils_test(type)
add_orion_utils_test(uninitialized)
