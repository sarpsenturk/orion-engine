cmake_minimum_required(VERSION 3.25)

set(ORION_VERSION_MAJOR 0 CACHE INTERNAL "Orion major version")
set(ORION_VERSION_MINOR 1 CACHE INTERNAL "Orion minor version")
set(ORION_VERSION_PATCH 0 CACHE INTERNAL "Orion patch version")
set(ORION_VERSION "${ORION_VERSION_MAJOR}.${ORION_VERSION_MINOR}.${ORION_VERSION_PATCH}" CACHE INTERNAL "Orion version string")

project(
    orion
    VERSION ${ORION_VERSION}
    LANGUAGES C CXX
)

option(
    ORION_BUILD_EXAMPLES
    "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

option(
    ORION_BUILD_TESTS
    "Enable building tests. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# Orion core engine library
add_library(orion "")
target_compile_features(orion PUBLIC cxx_std_20)
target_include_directories(orion PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Engine core sources
add_subdirectory(src)
add_subdirectory(include)

# Engine tests
if (ORION_BUILD_TESTS)
    include(CTest)
    add_subdirectory(tests)
endif()

# Example executables
if (ORION_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

