# Create interface library to be consumed by dynamic library implementations
add_library(orion-renderapi "")
add_library(orion::renderapi ALIAS orion-renderapi)

# Link with dependencies
target_link_libraries(
        orion-renderapi PUBLIC
        orion::core orion::utils orion::math
)

# Create the export headers
include(GenerateExportHeader)

# Set include directories
target_include_directories(orion-renderapi PUBLIC include/)

# Create file set for public headers
target_sources(orion-renderapi PUBLIC FILE_SET public_headers TYPE HEADERS)

# Add sources
add_subdirectory(include/orion-renderapi)
add_subdirectory(src)

# Add enabled builtin implementations
if (ORION_RENDER_BACKEND_VULKAN)
    add_subdirectory(vulkan)
endif ()
