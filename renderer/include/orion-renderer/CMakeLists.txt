# Set public headers
target_sources(
        orion-renderer PUBLIC
        FILE_SET public_headers
        FILES
        camera.h
        colors.h
        mesh.h
        renderer.h
        shader.h
        shader_compiler.h
        sprite_renderer.h
        vertex.h
)
