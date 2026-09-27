# Build imgui as a separate library
add_library(imgui
	"imgui.h"
	"imgui.cpp"
	"imgui_draw.cpp"
	"imgui_tables.cpp"
	"imgui_widgets.cpp"
	"imgui_demo.cpp"
	"imgui_impl_glfw.h" "imgui_impl_glfw.cpp"
	"imgui_impl_vulkan.h" "imgui_impl_vulkan.cpp"
)
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(imgui PRIVATE glfw Vulkan::Headers Vulkan::volk)