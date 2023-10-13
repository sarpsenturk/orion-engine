find_package(GTest REQUIRED)

function(add_orion_math_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} orion::math GTest::gtest_main)
    gtest_discover_tests(${name})
endfunction()


add_orion_math_test(abs)
add_orion_math_test(sqrt)
add_orion_math_test(vector)
add_orion_math_test(matrix)
add_orion_math_test(projection)
add_orion_math_test(angles)
add_orion_math_test(trig)
add_orion_math_test(transformation)
