add_library(imgui STATIC
	imgui.cpp
	imgui_draw.cpp
	imgui_tables.cpp
	imgui_widgets.cpp
	imgui_demo.cpp
	imgui_impl_glfw.cpp
	imgui_impl_vulkan.cpp
)
target_compile_features(imgui PUBLIC cxx_std_11)
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(imgui PRIVATE glfw Vulkan::Headers Vulkan::volk)
target_compile_definitions(imgui
	PRIVATE
		IMGUI_IMPL_VULKAN_USE_VOLK
		IMGUI_IMPL_VULKAN_VOLK_FILENAME="Volk/volk.h"
)
