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

# Third-party dependencies
find_package(spdlog 1.17.0 CONFIG REQUIRED)
find_package(glfw3 3.5.1 CONFIG REQUIRED)
find_package(Vulkan COMPONENTS volk REQUIRED) # Required VulkanSDK to be installed on the system
find_program(slangc_EXE slangc REQUIRED)
find_package(glm 1.0.3 CONFIG REQUIRED)

# Disable modules
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Engine core source
add_subdirectory(engine)

# Example executables
if (ORION_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()