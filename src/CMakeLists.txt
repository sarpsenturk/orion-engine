target_sources(
	orion
	PRIVATE
	orion.cpp
	application.cpp
	log.cpp
	assert.cpp

	platform/platform_glfw.cpp
	platform/rhi_vulkan.cpp

	rhi/rhi_instance.cpp
	rhi/rhi_device.cpp

	renderer/renderer.cpp
)
