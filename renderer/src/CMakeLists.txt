# Set private source and header files
target_sources(
        orion-renderer PRIVATE
        camera.cpp
        imgui_impl_orion.h imgui_impl_orion.cpp
        mesh.cpp
        render_window.cpp
        renderer.cpp
        shader.cpp
)
