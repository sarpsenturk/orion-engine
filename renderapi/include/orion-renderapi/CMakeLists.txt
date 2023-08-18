# Set interface headers
target_sources(
        orion-renderapi PUBLIC
        FILE_SET public_headers
        FILES
        command.h
        handles.h
        render_backend.h
        render_device.h
        types.h
)
