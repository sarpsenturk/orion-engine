# Set public header sources
target_sources(
        orion-core PUBLIC
        FILE_SET public_headers
        FILES
        clock.h
        defs.h
        dl_lib.h
        event.h
        exception.h
        handle.h
        input.h
        log.h
        platform.h
        types.h
        window.h
)

# Platform specific implementation headers
add_subdirectory(platform)
