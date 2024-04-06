# Set interface headers
target_sources(
        orion-renderapi PUBLIC
        FILE_SET public_headers
        FILES
        buffer.h
        defs.h
        device_resource.h
        handles.h
        render_backend.h
        render_command.h
        render_device.h
        shader_reflection.h
        swapchain.h
)
