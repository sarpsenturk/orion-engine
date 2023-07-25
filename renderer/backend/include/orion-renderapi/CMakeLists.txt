# Set interface headers
target_sources(
        orion-renderapi PUBLIC
        FILE_SET public_headers
        FILES
        barrier.h
        buffer.h
        command.h
        descriptor.h
        framebuffer.h
        handles.h
        pipeline.h
        render_backend.h
        render_device.h
        render_pass.h
        shader.h
        swapchain.h
        types.h
)
