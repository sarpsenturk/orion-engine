find_package(Vulkan REQUIRED COMPONENTS dxc volk)

target_link_libraries(orion PRIVATE Vulkan::Headers Vulkan::dxc_lib Vulkan::volk)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(vulkan_platform_impl vulkan_win32_impl.cpp)
else ()
    message(FATAL_ERROR "unknown/unsupported platform for Vulkan")
endif ()

target_sources(orion PRIVATE
        volk_impl.cpp
        vulkan_backend.h vulkan_backend.cpp
        vulkan_device.h vulkan_device.cpp
        vulkan_error.h vulkan_error.cpp
        vulkan_platform.h ${vulkan_platform_impl}
        vulkan_queue.h vulkan_queue.cpp
        vulkan_shader.h vulkan_shader.cpp
        vulkan_swapchain.h vulkan_swapchain.cpp
)
