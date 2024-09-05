find_package(directx-dxc CONFIG REQUIRED)
target_link_libraries(orion PRIVATE D3D12.lib Dxgi.lib Microsoft::DirectXShaderCompiler Microsoft::DXIL)

target_sources(orion PRIVATE
        d3d12_backend.h d3d12_backend.cpp
        d3d12_command.h d3d12_command.cpp
        d3d12_conversion.h d3d12_conversion.cpp
        d3d12_device.h d3d12_device.cpp
        d3d12_handle.h d3d12_handle.cpp
        d3d12_queue.h d3d12_queue.cpp
        d3d12_shader.h d3d12_shader.cpp
        d3d12_swapchain.h d3d12_swapchain.cpp
        orion_dx12.h orion_dx12.cpp
)
