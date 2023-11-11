# Set private header and sources
target_sources(
        orion-core PRIVATE
        clock.cpp
        dyn_lib.cpp
        filesystem.cpp
        input.cpp
        log.cpp
        window.cpp
)

# Platform specific implementation sources
add_subdirectory(platform)
