function(AddGTest)
    set(options "")
    set(one_value_args NAME FILENAME)
    set(multi_value_args DEPS)
    cmake_parse_arguments(
            GTEST_ADD_TEST
            "${options}"
            "${one_value_args}"
            "${multi_value_args}"
            ${ARGN}
    )
    add_executable(${GTEST_ADD_TEST_NAME} ${GTEST_ADD_TEST_FILENAME})
    target_link_libraries(${GTEST_ADD_TEST_NAME} PRIVATE GTest::gtest_main ${GTEST_ADD_TEST_DEPS})
    gtest_discover_tests(${GTEST_ADD_TEST_NAME})
endfunction()