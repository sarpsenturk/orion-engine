# Set private source and header files
target_sources(
        orion-renderer PRIVATE
        camera.cpp
        frame.cpp
        imgui_impl_orion.h imgui_impl_orion.cpp
        quad_renderer.cpp
        renderer.cpp
        shader.cpp
)
