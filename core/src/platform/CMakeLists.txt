# Add the appropriate directory to sources
if (ORION_PLATFORM_WINDOWS)
    add_subdirectory(win32)
elseif (ORION_PLATFORM_MACOS)
    add_subdirectory(unix)
    add_subdirectory(macos)
endif ()
