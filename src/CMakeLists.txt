target_sources(
        orion PRIVATE
        application.cpp
        window.cpp

        renderapi/buffer.cpp
        renderapi/format.cpp
        renderapi/pipeline.cpp
        renderapi/render_backend.cpp
        renderapi/render_command.cpp
        renderapi/render_device.cpp
        renderapi/render_queue.cpp
        renderapi/shader.cpp
        renderapi/swapchain.cpp
        renderapi/sync.cpp
)

add_subdirectory(renderapi/vulkan)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(win32)
    add_subdirectory(renderapi/d3d12)
else ()
    message(FATAL_ERROR "unknown/unsupported platform for implementation")
endif ()

# Configure file
configure_file(config.h.in ${PROJECT_BINARY_DIR}/conf/orion/config.h)
