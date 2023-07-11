# Set private header and sources
target_sources(
        orion-core PRIVATE
        clock.cpp
        dl_lib.cpp
        input.cpp
        window.cpp
)

# Platform specific implementation sources
add_subdirectory(platform)
