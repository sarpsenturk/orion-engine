# Set private headers and source files
target_sources(
        orion-core PRIVATE
        win32_clock.cpp
        win32_dl_lib.cpp
        win32_filesystem.cpp
        win32_input.cpp
        win32_platform.cpp
        win32_window.cpp
)
