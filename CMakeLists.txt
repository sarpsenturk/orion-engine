cmake_minimum_required(VERSION 3.21)

# Project configuration options
option(ORION_ENGINE_DEVELOPER_MODE "Enable developer mode" ON)
option(ORION_ENGINE_SANDBOX "Build the sandbox project" ${ORION_ENGINE_DEVELOPER_MODE})
option(ORION_ENGINE_TEST "Build tests for the orion engine" ${ORION_ENGINE_DEVELOPER_MODE})

# Enable testing feature with vcpkg
if (ORION_ENGINE_TEST)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif ()

# Detect current platform
include(cmake/platform.cmake)

project(orion-engine
        VERSION 0.1.0
        DESCRIPTION "A game engine written in C++ 20"
        )

set(ORION_ENGINE_TOP_LEVEL ${PROJECT_IS_TOP_LEVEL})

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
        <type_traits>
        )

# Create a base target for common properties and options
add_library(orion-base INTERFACE "")
target_compile_features(orion-base INTERFACE cxx_std_20)
target_link_libraries(orion-base INTERFACE orion-pch)
target_include_directories(orion-base INTERFACE ${CMAKE_BINARY_DIR}/include)

option(ORION_ENGINE_SANITIZE_ADDRESS "Enable address sanitizer" ${ORION_ENGINE_DEVELOPER_MODE})
if (ORION_ENGINE_SANITIZE_ADDRESS)
    target_compile_options(orion-base INTERFACE ${CXX_SANITIZE_ADDRESS})
endif ()

option(ORION_ENGINE_SANITIZE_UNDEFINED "Enable undefined behaviour sanitizer" ${ORION_ENGINE_DEVELOPER_MODE})
if (ORION_ENGINE_SANITIZE_UNDEFINED )
    target_compile_options(orion-base INTERFACE ${CXX_SANITIZE_UNDEFINED})
endif ()


# Enable/disable testing
if (ORION_ENGINE_TEST)
    enable_testing()
endif ()

# Add sources for submodules
add_subdirectory(utils)
add_subdirectory(math)
add_subdirectory(core)
add_subdirectory(renderer)
add_subdirectory(engine)

# Enable/disable sandbox
if (ORION_ENGINE_SANDBOX)
    add_subdirectory(sandbox)
endif ()
