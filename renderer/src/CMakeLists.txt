# Set private source and header files
target_sources(
        orion-renderer PRIVATE
        buffer.cpp
        camera.cpp
        frame.cpp
        imgui.cpp
        imgui_impl_orion.h imgui_impl_orion.cpp
        material.cpp
        mesh.cpp
        render_context.cpp
        renderer.cpp
        shader.cpp
        texture.cpp
)
