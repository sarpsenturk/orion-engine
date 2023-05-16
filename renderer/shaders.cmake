set(DXC_PATH "" CACHE PATH "Path to the DirectXShaderCompiler root directory")
message(STATUS "DirectXShaderCompiler path: ${DXC_PATH}")
if (NOT DXC_PATH)
    message(SEND_ERROR "DirectXShaderCompiler directory not set.")
endif ()

cmake_path(APPEND DXC_LIBDIR "${DXC_PATH}" "lib")
cmake_path(APPEND DXC_BINDIR "${DXC_PATH}" "bin")
cmake_path(APPEND DXC_INCLUDEDIR "${DXC_PATH}" "include")

# Create imported library
add_library(dxcompiler SHARED IMPORTED)

# Set the include directory
set_target_properties(dxcompiler PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${DXC_PATH}/include")

# Set imported library locations
if (WIN32)
    set_target_properties(dxcompiler PROPERTIES
            IMPORTED_LOCATION "${DXC_BINDIR}/dxcompiler.dll"
            IMPORTED_IMPLIB "${DXC_LIBDIR}/dxcompiler.lib"
            )
else ()
    set_target_properties(dxcompiler PROPERTIES IMPORTED_LOCATION "${DXC_LIBDIR}/libdxcompiler.so")
endif ()
