# Set vulkan platform implementation
if (ORION_PLATFORM_WINDOWS)
    set(vulkan_platform_impl vulkan_win32.cpp)
else ()
    message(FATAL_ERROR "Unsupported platform for the Vulkan backend")
endif ()

# Create the vulkan implementation of render-backend interface
add_library(
        ${ORION_VULKAN_MODULE} MODULE
        vma_impl.cpp
        volk_impl.cpp
        vulkan_backend.h vulkan_backend.cpp
        vulkan_command.h vulkan_command.cpp
        vulkan_conversion.h vulkan_conversion.cpp
        vulkan_device.h vulkan_device.cpp
        vulkan_headers.h
        vulkan_platform.h ${vulkan_platform_impl}
        vulkan_queue.h vulkan_queue.cpp
        vulkan_reflection.h vulkan_reflection.cpp
        vulkan_renderpass.h vulkan_renderpass.cpp
        vulkan_resource.h vulkan_resource.cpp
        vulkan_swapchain.h vulkan_swapchain.cpp
        vulkan_types.h vulkan_types.cpp
)

# Set library to export
target_compile_definitions(${ORION_VULKAN_MODULE} PRIVATE ORION_RENDER_BACKEND_EXPORT)

# Find vulkan dependency
find_package(Vulkan REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(unofficial-spirv-reflect CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)

# Link with interface
target_link_libraries(
        ${ORION_VULKAN_MODULE}
        PRIVATE
        orion::renderapi
        Vulkan::Headers
        unofficial::vulkan-memory-allocator::vulkan-memory-allocator
        unofficial::spirv-reflect::spirv-reflect
        volk::volk_headers
)

# Precompiled headers
target_precompile_headers(orion-renderapi REUSE_FROM orion-pch)

# Set private include directory
target_include_directories(${ORION_VULKAN_MODULE} PRIVATE .)

# Create alias
add_library(orion::vulkan ALIAS ${ORION_VULKAN_MODULE})

# Output module to bin directory
set_target_properties(
        ${ORION_VULKAN_MODULE} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_dependencies(orion-renderapi-builtin ${ORION_VULKAN_MODULE})
