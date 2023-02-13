# Module specific options
option(ORION_RENDERER_TEST "Enable tests for the orion renderer" ${ORION_ENGINE_TEST})

# Builtin backend targets
option(ORION_RENDER_BACKEND_VULKAN "Enable the Vulkan backend for the orion renderer" ON)

# Create the renderer library
add_library(orion-renderer "")

# Create alias for consistency
add_library(orion::renderer ALIAS orion-renderer)

# Set include directories
target_include_directories(orion-renderer PUBLIC include/)

# Link with dependencies
target_link_libraries(
        orion-renderer PUBLIC
        orion::core
        orion::math
        orion::utils
)

# Create file set for public headers
target_sources(orion-renderer PUBLIC FILE_SET public_headers TYPE HEADERS)

# Add sources
add_subdirectory(include/orion-renderer)
add_subdirectory(src)

# Enable/disable tests
if (ORION_RENDERER_TEST)
    add_subdirectory(tests)
endif ()
