# Set private source and header files
target_sources(
        orion-renderer PRIVATE
        camera.cpp
        frame.cpp
        imgui.cpp
        imgui_impl_orion.h imgui_impl_orion.cpp
        quad_renderer.cpp
        render_context.cpp
        render_window.cpp
        renderer.cpp
        shader.cpp
)
