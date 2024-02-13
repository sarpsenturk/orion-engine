cmake_minimum_required(VERSION 3.21)

# Project configuration options
option(ORION_DEVELOPER_MODE "Enable developer mode" ON)
option(ORION_SANDBOX "Build the sandbox project" ${ORION_DEVELOPER_MODE})
option(ORION_TEST "Build tests for the orion engine" ${ORION_DEVELOPER_MODE})

# Enable/disable builtin backends
option(ORION_ENABLE_VULKAN "Enable the Vulkan backend for the orion renderer" ON)

include(CMakeDependentOption)
cmake_dependent_option(ORION_USE_VULKAN_DXC "Use DXC from the Vulkan SDK" ON ORION_ENABLE_VULKAN OFF)
if (NOT ORION_USE_VULKAN_DXC)
    list(APPEND VCPKG_MANIFEST_FEATURES "directx-dxc")
endif ()

# Enable testing feature with vcpkg
if (ORION_TEST)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif ()
if (ORION_ENABLE_VULKAN)
    list(APPEND VCPKG_MANIFEST_FEATURES "vulkan")
endif ()

# Add cmake path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

project(orion-engine
        VERSION 0.1.0
        DESCRIPTION "A game engine written in C++ 20"
        LANGUAGES CXX
)

# Project wide configuration
set(CMAKE_CXX_STANDARD 20)
include_directories(${CMAKE_BINARY_DIR}/include)
add_compile_definitions(
        ORION_VERSION="${PROJECT_VERSION}"
        ORION_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        ORION_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        ORION_VERSION_PATCH=${PROJECT_VERSION_PATCH}

        ORION_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
        $<$<CONFIG:Debug>:ORION_BUILD_DEBUG>
        $<$<CONFIG:RelWithDebInfo>:ORION_BUILD_RELEASE>
        $<$<CONFIG:Release>:ORION_BUILD_DIST>

        $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>
        $<$<CONFIG:RelWithDebInfo>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
        $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_WARN>

        ORION_PLATFORM="${ORION_PLATFORM}"
        $<$<BOOL:${ORION_PLATFORM_WINDOWS}>:ORION_PLATFORM_WINDOWS>
        $<$<BOOL:${ORION_PLATFORM_LINUX}>:ORION_PLATFORM_LINUX>
        $<$<BOOL:${ORION_PLATFORM_MACOS}>:ORION_PLATFORM_MACOS>
)

option(ORION_SANITIZE_ADDRESS "Enable address sanitizer" ${ORION_DEVELOPER_MODE})
if (ORION_SANITIZE_ADDRESS)
    if ("${CXX_SANITIZE_ADDRESS}" STREQUAL "")
        message(AUTHOR_WARNING "Current platform/compiler doesn't support address sanitizer. Disabling.")
        set(ORION_SANITIZE_ADDRESS OFF CACHE BOOL "Enable address sanitizer" FORCE)
    endif ()
    add_compile_options(${CXX_SANITIZE_ADDRESS})
endif ()

option(ORION_SANITIZE_UNDEFINED "Enable undefined behaviour sanitizer" ${ORION_DEVELOPER_MODE})
if (ORION_SANITIZE_UNDEFINED)
    if ("${CXX_SANITIZE_UNDEFINED}" STREQUAL "")
        message(AUTHOR_WARNING "Current platform/compiler doesn't support undefined behaviour sanitizer. Disabling.")
        set(ORION_SANITIZE_UNDEFINED OFF CACHE BOOL "Enable undefined behaviour sanitizer" FORCE)
    endif ()
    add_compile_options(${CXX_SANITIZE_UNDEFINED})
endif ()

# Create a target for precompiled headers
add_library(orion-pch pch.h pch.cpp)
target_precompile_headers(orion-pch PUBLIC pch.h)
add_library(orion::pch ALIAS orion-pch)

# Enable/disable testing
if (ORION_TEST)
    enable_testing()
endif ()

# Add extra external dependencies
if (ORION_PLATFORM_MACOS)
    add_subdirectory(external/metal-cpp)
endif ()

# Add sources for submodules
add_subdirectory(utils)
add_subdirectory(math)
add_subdirectory(core)
add_subdirectory(shader-compiler)
add_subdirectory(assets)
add_subdirectory(renderapi)
add_subdirectory(renderer)
add_subdirectory(scene)
add_subdirectory(engine)

# Enable/disable sandbox
if (ORION_SANDBOX)
    add_subdirectory(sandbox)
endif ()
