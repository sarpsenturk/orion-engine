# Set vulkan platform implementation
if (ORION_PLATFORM_WINDOWS)
    set(vulkan_platform_impl vulkan_win32.cpp)
else ()
    message(FATAL_ERROR "Unsupported platform for the Vulkan backend")
endif ()

# Create the vulkan implementation of render-backend interface
add_library(
        ${ORION_VULKAN_MODULE} SHARED
        vma_impl.cpp
        vulkan_backend.h vulkan_backend.cpp
        vulkan_conversion.h
        vulkan_device.h vulkan_device.cpp
        vulkan_headers.h
        vulkan_platform.h ${vulkan_platform_impl}
        vulkan_store.h
        vulkan_types.h vulkan_types.cpp
)

# Set library to export
target_compile_definitions(${ORION_VULKAN_MODULE} PRIVATE ORION_RENDER_BACKEND_EXPORT)

# Find vulkan dependency
find_package(Vulkan REQUIRED)

# Link with interface
target_link_libraries(
    ${ORION_VULKAN_MODULE} 
    PRIVATE 
    orion::renderapi 
    Vulkan::Vulkan
)

# Precompiled headers
target_precompile_headers(orion-renderapi REUSE_FROM orion-pch)

# Set private include directory
target_include_directories(${ORION_VULKAN_MODULE} PRIVATE .)

# Create alias for ease of use
add_library(orion::vulkan ALIAS ${ORION_VULKAN_MODULE})

# Add dependency from renderer to the vulkan library
add_dependencies(orion-renderer orion::vulkan)

set_target_properties(${ORION_VULKAN_MODULE} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
