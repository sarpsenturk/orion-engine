# Module specific options
option(ORION_RENDERER_TEST "Enable tests for the orion renderer" ${ORION_ENGINE_DEVELOPER_MODE})

# Builtin backend targets
option(ORION_RENDER_BACKEND_VULKAN "Enable the Vulkan backend for the orion renderer" ON)
set(ORION_VULKAN_MODULE "orion-vulkan" CACHE STRING "Name of the Vulkan render backend shared library")

# Create the renderer library
add_library(orion-renderer "")

# Create alias for consistency
add_library(orion::renderer ALIAS orion-renderer)

# Set include directories
target_include_directories(orion-renderer PUBLIC include/)

# Backend interface library
add_subdirectory(backend)

# Shader compilation library
find_package(directx-dxc CONFIG REQUIRED)

# Link with dependencies
target_link_libraries(
        orion-renderer
        PUBLIC
        orion::core
        orion::math
        orion::utils
        orion::renderapi
        PRIVATE
        Microsoft::DirectXShaderCompiler
)

# Create file set for public headers
target_sources(orion-renderer PUBLIC FILE_SET public_headers TYPE HEADERS)

# Add configuration file
set(ORION_ASSET_DIR ${CMAKE_SOURCE_DIR}/assets)
set(ORION_SHADER_DIR ${ORION_ASSET_DIR}/shaders)
configure_file(config.h.in ${CMAKE_BINARY_DIR}/include/orion-renderer/config.h)

# Add sources
add_subdirectory(include/orion-renderer)
add_subdirectory(src)

# Enable/disable tests
if (ORION_RENDERER_TEST AND ORION_ENGINE_TEST)
    add_subdirectory(tests)
endif ()
