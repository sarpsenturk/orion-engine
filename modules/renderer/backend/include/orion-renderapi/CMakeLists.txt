# Set interface headers
target_sources(
        orion-render-backend PUBLIC
        FILE_SET public_headers
        FILES
        buffer.h
        command.h
        handles.h
        pipeline.h
        render_backend.h
        render_context.h
        render_device.h
        shader.h
        swapchain.h
        types.h
)
