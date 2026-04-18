target_sources(
    orion
    PRIVATE
    orion.cpp
    application.cpp
    log.cpp
    debug.cpp
    window.cpp
    event.cpp

    platform/platform_glfw.hpp
    platform/platform_glfw.cpp

    renderer/renderer.cpp
    renderer/render_graph.cpp
    renderer/pipeline.cpp
    renderer/vulkan_impl.hpp
    renderer/vulkan_impl.cpp
    renderer/imgui_context.hpp
    renderer/imgui_context.cpp
)

# Compile glsl shaders to SPIR-V
add_subdirectory(shaders)
