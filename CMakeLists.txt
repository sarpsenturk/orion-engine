cmake_minimum_required(VERSION 3.21)

project(orion-engine
        VERSION 0.1.0
        DESCRIPTION "A game engine written in C++ 20"
        )

set(ORION_ENGINE_TOP_LEVEL ${PROJECT_IS_TOP_LEVEL})

# Project configuration options
option(ORION_ENGINE_SANDBOX "Build the sandbox project" ${ORION_ENGINE_TOP_LEVEL})
option(ORION_ENGINE_TEST "Build tests for the orion engine" ${ORION_ENGINE_TOP_LEVEL})

# Create a target for precompiled headers
add_library(orion-pch INTERFACE "")
target_precompile_headers(orion-pch INTERFACE
        <utility>
        <version>
        <memory>
        <cstdint>
        <string>
        <string_view>
        <array>
        <span>
        <vector>
        <algorithm>
        )

# Create a base target for common properties and options
add_library(orion-base INTERFACE "")
target_compile_features(orion-base INTERFACE cxx_std_20)
target_link_libraries(orion-base INTERFACE orion-pch)
target_include_directories(orion-base INTERFACE ${CMAKE_BINARY_DIR}/include)

# Enable/disable testing
if (ORION_ENGINE_TEST)
    enable_testing()
endif ()

# Add sources for submodules
add_subdirectory(modules/utils)
add_subdirectory(modules/math)
add_subdirectory(modules/core)
add_subdirectory(modules/engine)

# Enable/disable sandbox
if (ORION_ENGINE_SANDBOX)
    add_subdirectory(sandbox)
endif ()
