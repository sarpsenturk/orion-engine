if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(vulkan_platform_impl vulkan_win32_impl.cpp)
else ()
    message(FATAL_ERROR "unknown/unsupported platform for Vulkan")
endif ()

find_package(Vulkan REQUIRED COMPONENTS volk)
find_package(directx-dxc CONFIG REQUIRED)

target_link_libraries(orion PRIVATE Vulkan::Headers Vulkan::volk Microsoft::DirectXShaderCompiler)

target_sources(orion PRIVATE
        volk_impl.cpp
        vulkan_backend.h vulkan_backend.cpp
        vulkan_command.h vulkan_command.cpp
        vulkan_conversion.h vulkan_conversion.cpp
        vulkan_device.h vulkan_device.cpp
        vulkan_error.h vulkan_error.cpp
        vulkan_handle.h vulkan_handle.cpp
        vulkan_platform.h ${vulkan_platform_impl}
        vulkan_queue.h vulkan_queue.cpp
        vulkan_shader.h vulkan_shader.cpp
        vulkan_swapchain.h vulkan_swapchain.cpp
)
