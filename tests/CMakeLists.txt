cmake_policy(SET CMP0135 NEW)
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

function(orion_add_gtest name src)
    add_executable(${name} ${src})
    target_link_libraries(${name} orion::orion GTest::gtest_main)
    gtest_discover_tests(${name})
endfunction()

orion_add_gtest(log log.cpp)
