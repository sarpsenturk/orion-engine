# Public headers
target_sources(
        orion-core PUBLIC
        FILE_SET public_headers
        FILES
        include/orion-win32/win32_dyn_lib.h
        include/orion-win32/win32_input.h
        include/orion-win32/win32_platform.h
        include/orion-win32/win32_window.h

        PRIVATE
        src/win32_clock.cpp
        src/win32_dyn_lib.cpp
        src/win32_input.cpp
        src/win32_platform.cpp
        src/win32_window.cpp
)

# Add include directory
target_include_directories(orion-core PUBLIC include)
