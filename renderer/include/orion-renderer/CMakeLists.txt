# Set public headers
target_sources(
        orion-renderer PUBLIC
        FILE_SET public_headers
        FILES
        camera.h
        mesh.h
        renderer.h
        shader.h
        shader_compiler.h
)
