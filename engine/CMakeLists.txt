
# Create the engine library
add_library(orion-engine "")

# Create alias for consistency
add_library(orion::engine ALIAS orion-engine)

# Set include directories
target_include_directories(orion-engine PUBLIC include/)

# Link with dependencies
target_link_libraries(
    orion-engine 
    PUBLIC 
    orion::core
    orion::renderer
)

# Precompiled headers
target_precompile_headers(orion-engine REUSE_FROM orion-pch)

# Create file set for public headers
target_sources(orion-engine PUBLIC FILE_SET public_headers TYPE HEADERS)

# Add sources
add_subdirectory(include/orion-engine)
add_subdirectory(src)

# Enable/disable tests
if (ORION_ENGINE_TESTS)
    add_subdirectory(tests)
endif ()
