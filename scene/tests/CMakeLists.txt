find_package(GTest REQUIRED)

function(add_orion_scene_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} orion::scene GTest::gtest_main)
    gtest_discover_tests(${name})
endfunction()

add_orion_scene_test(components)
