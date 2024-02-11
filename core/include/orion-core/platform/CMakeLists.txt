# Add the appropriate directory to sources
if (ORION_PLATFORM_WINDOWS)
    add_subdirectory(win32)
elseif (ORION_PLATFORM_LINUX)
    add_subdirectory(linux)
endif ()

if (ORION_UNIX_LIKE)
    add_subdirectory(unix)
endif ()
