# Create interface library to be consumed by dynamic library implementations
add_library(orion-render-backend INTERFACE "")

# Link with dependencies
target_link_libraries(
        orion-render-backend INTERFACE
        orion::core orion::utils orion::math
)

# Create the export headers
include(GenerateExportHeader)

# Set include directories
target_include_directories(orion-render-backend INTERFACE include/)

# Create file set for public headers
target_sources(orion-render-backend INTERFACE FILE_SET public_headers TYPE HEADERS)

# Add headers
add_subdirectory(include/orion-renderer)

# Add enabled builtin implementations
if (ORION_RENDER_BACKEND_VULKAN)
    add_subdirectory(vulkan)
endif ()
