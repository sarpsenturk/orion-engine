# Set public header sources
target_sources(
        orion-core PUBLIC
        FILE_SET public_headers
        FILES
        clock.h
        event.h
        exception.h
        filesystem.h
        handle.h
        log.h
        module.h
        version.h
        window.h
)
