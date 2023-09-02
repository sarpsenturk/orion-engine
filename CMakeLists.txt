cmake_minimum_required(VERSION 3.21)

# Project configuration options
option(ORION_ENGINE_DEVELOPER_MODE "Enable developer mode" ON)
option(ORION_ENGINE_SANDBOX "Build the sandbox project" ${ORION_ENGINE_DEVELOPER_MODE})
option(ORION_ENGINE_TEST "Build tests for the orion engine" ${ORION_ENGINE_DEVELOPER_MODE})

# Enable testing feature with vcpkg
if (ORION_ENGINE_TEST)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif ()


project(orion-engine
        VERSION 0.1.0
        DESCRIPTION "A game engine written in C++ 20"
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

        ORION_PLATFORM="${ORION_PLATFORM}"
        $<$<BOOL:${ORION_PLATFORM_WINDOWS}>:ORION_PLATFORM_WINDOWS>
        $<$<BOOL:${ORION_PLATFORM_LINUX}>:ORION_PLATFORM_LINUX>
)

option(ORION_ENGINE_SANITIZE_ADDRESS "Enable address sanitizer" ${ORION_ENGINE_DEVELOPER_MODE})
if (ORION_ENGINE_SANITIZE_ADDRESS)
    if ("${CXX_SANITIZE_ADDRESS}" STREQUAL "")
        message(AUTHOR_WARNING "Current platform/compiler doesn't support address sanitizer even though it's enabled")
    endif ()
    add_compile_options(${CXX_SANITIZE_ADDRESS})
endif ()

option(ORION_ENGINE_SANITIZE_UNDEFINED "Enable undefined behaviour sanitizer" ${ORION_ENGINE_DEVELOPER_MODE})
if (ORION_ENGINE_SANITIZE_UNDEFINED)
    if ("${CXX_SANITIZE_UNDEFINED}" STREQUAL "")
        message(AUTHOR_WARNING "Current platform/compiler doesn't support undefined behaviour sanitizer even though it's enabled")
    endif ()
    add_compile_options(${CXX_SANITIZE_UNDEFINED})
endif ()

# Create a target for precompiled headers
add_library(orion-pch pch.h pch.cpp)
target_precompile_headers(orion-pch PUBLIC pch.h)
add_library(orion::pch ALIAS orion-pch)

# Enable/disable testing
if (ORION_ENGINE_TEST)
    enable_testing()
endif ()

# Add sources for submodules
add_subdirectory(utils)
add_subdirectory(math)
add_subdirectory(core)
add_subdirectory(renderapi)
add_subdirectory(renderer)
add_subdirectory(scene)
add_subdirectory(engine)

# Enable/disable sandbox
if (ORION_ENGINE_SANDBOX)
    add_subdirectory(sandbox)
endif ()
