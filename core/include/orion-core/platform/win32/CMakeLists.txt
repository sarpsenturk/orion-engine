# Set public header sources
target_sources(
        orion-core PUBLIC
        FILE_SET public_headers
        FILES
        win32_dl_lib.h
        win32_headers.h
        win32_input.h
        win32_window.h
)