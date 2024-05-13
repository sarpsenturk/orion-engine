# Set public headers
target_sources(
        orion-renderer PUBLIC
        FILE_SET public_headers
        FILES
        colors.h
        effect.h
        mesh.h
        renderer.h
)
