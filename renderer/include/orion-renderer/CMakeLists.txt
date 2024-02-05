# Set public headers
target_sources(
        orion-renderer PUBLIC
        FILE_SET public_headers
        FILES
        camera.h
        colors.h
        frame.h
        quad_renderer.h
        renderer.h
        shader.h
)
