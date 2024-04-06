# Set public headers
target_sources(
        orion-renderer PUBLIC
        FILE_SET public_headers
        FILES
        camera.h
        colors.h
        frame.h
        imgui.h
        quad_renderer.h
        render_context.h
        render_window.h
        renderer.h
        shader.h
)
