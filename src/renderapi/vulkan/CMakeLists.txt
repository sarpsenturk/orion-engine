find_package(VulkanHeaders CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)
target_link_libraries(orion PRIVATE Vulkan::Headers volk::volk_headers)

target_sources(orion PRIVATE
        volk_impl.cpp
        vulkan_backend.h vulkan_backend.cpp
        vulkan_error.h vulkan_error.cpp
)
