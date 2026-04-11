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

# Third party dependencies
find_package(spdlog 1.17.0 CONFIG REQUIRED)
find_package(glfw3 3.4 CONFIG REQUIRED)
find_package(tl-expected 1.3.1 CONFIG REQUIRED)
find_package(volk 1.4.328.0 CONFIG REQUIRED)
find_package(VulkanUtilityLibraries CONFIG REQUIRED)
find_package(VulkanMemoryAllocator 3.3.0 CONFIG REQUIRED)

# Vendored third party libraries
add_subdirectory(thirdparty/imgui)

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

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/include/orion/config.h)

# Orion core engine library
add_library(orion "")
target_compile_features(orion PUBLIC cxx_std_20)
target_include_directories(
    orion
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)
target_compile_options(
    orion
    PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /external:anglebrackets /external:W0>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wconversion -pedantic -Werror>
)
target_link_libraries(
    orion
    PUBLIC
    spdlog::spdlog
    tl::expected
    imgui
    volk::volk_headers
    PRIVATE
    glfw
    volk::volk # Can link directly, don't need platform defines with glfw
    Vulkan::UtilityHeaders
    GPUOpen::VulkanMemoryAllocator
)
target_compile_definitions(
    orion
    PUBLIC
    $<$<CONFIG:Debug>:ORION_BUILD_DEBUG>
)

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

