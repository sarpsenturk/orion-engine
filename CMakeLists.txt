cmake_minimum_required(VERSION 3.12)

project(orion-engine
        DESCRIPTION "A cross platform game engine written in C++ 20"
        VERSION 0.1.0
        LANGUAGES CXX)

# External dependencies
add_subdirectory(external/spdlog)

add_library(orion-engine STATIC
        # Public headers
        include/orion/orion.h

        include/orion/core/application.h
        include/orion/core/entry_point.h

        # Source files
        src/core/application.cpp
        )

target_compile_features(orion-engine PUBLIC cxx_std_20)
target_link_libraries(orion-engine
        PUBLIC
        spdlog
        )
target_include_directories(orion-engine PUBLIC include/)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(EnableGoogleTest)
EnableGoogleTest(ORION_ENGINE)

option(ORION_BUILD_SANDBOX "Build the Orion sandbox project" OFF)
if (ORION_BUILD_SANDBOX)
    add_subdirectory(Sandbox)
endif ()
