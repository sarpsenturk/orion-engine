# Set public header sources
target_sources(
        orion-core PUBLIC
        FILE_SET public_headers
        FILES
        event.h
        platform.h
        window.h
)

# Platform specific implementation headers
add_subdirectory(platform)
