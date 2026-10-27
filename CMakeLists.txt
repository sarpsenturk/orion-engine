cmake_minimum_required(VERSION 3.30)

# Orion project declaration
project(
	orion
	VERSION 0.1.0
	LANGUAGES CXX
)

# Project configuration options
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
set(
	ORION_SHADER_OUTPUT_DIR
	"${CMAKE_CURRENT_BINARY_DIR}/shaders" 
	CACHE PATH
	"Directory for compiled shader (SPIR-V) output"
)

# Third-party dependencies
find_package(spdlog 1.17.0 CONFIG REQUIRED)
find_package(glfw3 3.5.1 CONFIG REQUIRED)
find_package(Vulkan COMPONENTS volk REQUIRED) # Requires Vulkan SDK to be installed on the system
find_program(slangc_EXE slangc REQUIRED)
find_package(glm 1.0.3 CONFIG REQUIRED)

# Disable modules
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if (ORION_BUILD_TESTS)
	enable_testing()
	include(cmake/gtest.cmake)
endif()

# Vendored dependencies
add_subdirectory(third_party)

# Engine core source
add_subdirectory(engine)

# Example executables
if (ORION_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
