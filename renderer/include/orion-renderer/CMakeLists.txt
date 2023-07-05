# Set public headers
target_sources(
        orion-renderer PUBLIC
        FILE_SET public_headers
        FILES
        camera.h
        renderer.h
        shader_compiler.h
)
