if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(vulkan_platform_impl vulkan_win32_impl.cpp)
else ()
    message(FATAL_ERROR "unknown/unsupported platform for Vulkan")
endif ()

find_package(Vulkan REQUIRED COMPONENTS volk)
find_package(directx-dxc CONFIG REQUIRED)

target_link_libraries(orion PRIVATE Vulkan::Headers Vulkan::volk Microsoft::DirectXShaderCompiler)

target_sources(orion PRIVATE
        vma_impl.cpp
        volk_impl.cpp
        vulkan_backend.hpp vulkan_backend.cpp
        vulkan_command.hpp vulkan_command.cpp
        vulkan_context.hpp vulkan_context.cpp
        vulkan_conversion.hpp vulkan_conversion.cpp
        vulkan_device.hpp vulkan_device.cpp
        vulkan_error.hpp vulkan_error.cpp
        vulkan_platform.hpp ${vulkan_platform_impl}
        vulkan_queue.hpp vulkan_queue.cpp
        vulkan_shader.hpp vulkan_shader.cpp
        vulkan_swapchain.hpp vulkan_swapchain.cpp
)
