# Set interface headers
target_sources(
        orion-render-backend PUBLIC
        FILE_SET public_headers
        FILES
        render_backend.h
        render_device.h
        types.h
)
