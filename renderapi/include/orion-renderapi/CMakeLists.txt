# Set interface headers
target_sources(
        orion-renderapi PUBLIC
        FILE_SET public_headers
        FILES
        blend.h
        buffer.h
        descriptor.h
        device_resource.h
        format.h
        handles.h
        image.h
        physical_device.h
        pipeline.h
        queue.h
        render_backend.h
        render_command.h
        render_device.h
        render_pass.h
        shader.h
        shader_reflection.h
        swapchain.h
        types.h
)
