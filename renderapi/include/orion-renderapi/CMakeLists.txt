# Set interface headers
target_sources(
        orion-renderapi PUBLIC
        FILE_SET public_headers
        FILES
        handles.h
        render_backend.h
        render_command.h
        render_device.h
        swapchain.h
        types.h
)
