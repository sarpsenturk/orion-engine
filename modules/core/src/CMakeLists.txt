# Set private header and sources
target_sources(
        orion-core PRIVATE
        window.cpp
)

# Platform specific implementation sources
add_subdirectory(platform)
