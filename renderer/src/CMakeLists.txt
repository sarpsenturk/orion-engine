# Set private source and header files
target_sources(
        orion-renderer PRIVATE
        dxc_include.h
        renderer.cpp
        shader_compiler.cpp
)
