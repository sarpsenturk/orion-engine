# Set private source and header files
target_sources(
        orion-renderer PRIVATE
        camera.cpp
        dxc_include.h
        imgui_impl_orion.h imgui_impl_orion.cpp
        mesh.cpp
        renderer.cpp
        shader_compiler.cpp
)
