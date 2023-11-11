# Set public header sources
target_sources(
        orion-core PUBLIC
        FILE_SET public_headers
        FILES
        clock.h
        dyn_lib.h
        event.h
        exception.h
        filesystem.h
        handle.h
        input.h
        log.h
        platform.h
        version.h
        window.h
)

# Platform specific implementation headers
add_subdirectory(platform)
