set(msvc_debug_flags "/WX")
set(msvc_warning_flags "/permissive-" "/W4" "/w14640" "/external:anglebrackets" "/external:W0")
set(msvc_flags "${msvc_warning_flags}" "$<$<CONFIG:Debug>:${msvc_debug_flags}>")

set(gcc_debug_flags "-Werror")
set(gcc_warning_flags "-Wall" "-Wextra" "-Wshadow" "-Wnon-virtual-dtor" "-pedantic")
set(gcc_flags "${gcc_warning_flags}" "$<$<CONFIG:Debug>:${gcc_debug_flags}>")

target_compile_options(orion PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,${msvc_flags},${gcc_flags}>)
