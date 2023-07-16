# Set interface headers
target_sources(
        orion-renderapi PUBLIC
        FILE_SET public_headers
        FILES
        buffer.h
        command.h
        descriptor.h
        framebuffer.h
        handles.h
        pipeline.h
        render_backend.h
        render_device.h
        render_pass.h
        render_target.h
        shader.h
        swapchain.h
        types.h
)
