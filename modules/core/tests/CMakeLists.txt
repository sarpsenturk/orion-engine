find_package(GTest REQUIRED)

function(add_orion_core_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} orion::core GTest::gtest_main)
    gtest_discover_tests(${name})
endfunction()

add_orion_core_test(event)
