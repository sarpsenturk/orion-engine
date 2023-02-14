# Create the vulkan implementation of render-backend interface
add_library(
        ${ORION_VULKAN_MODULE} SHARED
        vulkan_backend.h vulkan_backend.cpp
        vulkan_headers.h
)

# Find vulkan dependency
find_package(Vulkan REQUIRED)

# Link with interface
target_link_libraries(${ORION_VULKAN_MODULE} PRIVATE orion-render-backend Vulkan::Vulkan)

# Set private include directory
target_include_directories(${ORION_VULKAN_MODULE} PRIVATE .)

# Create alias for ease of use
add_library(orion::vulkan ALIAS ${ORION_VULKAN_MODULE})

# Add dependency from renderer to the vulkan library
add_dependencies(orion-renderer orion::vulkan)
