# Set public headers
target_sources(
        orion-renderer PUBLIC
        FILE_SET public_headers
        FILES
        camera.h
        colors.h
        effect.h
        imgui.h
        material.h
        mesh.h
        render_target.h
        render_window.h
        renderer.h
        texture.h
        types.h
        render_context.h
)
