if (ORION_USE_VULKAN_DXC)
    find_package(Vulkan REQUIRED)
    add_library(dxc_lib SHARED IMPORTED)
    set_target_properties(
            dxc_lib PROPERTIES
            # TODO: Find a more consisten way to do this
            IMPORTED_LOCATION "/usr/local/lib/libdxcompiler.dylib"
            INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIR}/dxc"
    )
else ()
    find_package(directx-dxc CONFIG REQUIRED)
    add_library(dxc_lib ALIAS Microsoft::DirectXShaderCompiler)
endif ()