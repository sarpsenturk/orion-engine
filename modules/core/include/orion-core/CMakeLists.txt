# Set public header sources
target_sources(
        orion-core PUBLIC
        FILE_SET public_headers
        FILES
        dl_lib.h
        event.h
        exception.h
        handle.h
        platform.h
        types.h
        window.h
)

# Platform specific implementation headers
add_subdirectory(platform)
