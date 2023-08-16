# Module specific options
option(ORION_RENDERER_TEST "Enable tests for the orion renderer" ${ORION_ENGINE_DEVELOPER_MODE})
option(ORION_RENDERER_HEADLESS "Build renderer in headless mode (disables support for surfaces etc.)" OFF)
set(ORION_FRAMES_IN_FLIGHT 2 CACHE STRING "Number of frames in flight")

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

# Imgui static library
find_package(imgui CONFIG REQUIRED)

# Create user imgui config file
set(imgui_config_file ${CMAKE_BINARY_DIR}/include/imconfig.h)
configure_file(imconfig.h.in ${imgui_config_file})
target_compile_definitions(orion-renderer PUBLIC IMGUI_USER_CONFIG="${imgui_config_file}")

# Link with dependencies
target_link_libraries(
        orion-renderer
        PUBLIC
        orion::core
        orion::math
        orion::utils
        orion::renderapi
        imgui::imgui
        Microsoft::DirectXShaderCompiler
)

# Precompiled headers
target_precompile_headers(orion-renderer REUSE_FROM orion-pch)

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
