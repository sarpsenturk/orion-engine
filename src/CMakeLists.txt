target_sources(
	orion
	PRIVATE
	orion.cpp
	application.cpp
	log.cpp
	assert.cpp

	platform/platform_glfw.hpp
	platform/platform_glfw.cpp
	platform/rhi_vulkan.cpp

	rhi/rhi.cpp
	rhi/device.cpp
	rhi/command.cpp
	rhi/swapchain.cpp

	renderer/renderer.cpp
)
