set(ORION_SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders CACHE INTERNAL "Orion shader binary directory")

# Find DirectXShaderCompiler from VulkanSDK
find_program(
	DXC_EXECUTABLE
	NAMES dxc
	PATHS $ENV{VULKAN_SDK}/Bin
	NO_DEFAULT_PATH
	REQUIRED
)

# Add shader for compilation
function(orion_add_shader input output profile)
	set(shader_src ${CMAKE_CURRENT_SOURCE_DIR}/${input})
	set(shader_output ${ORION_SHADER_DIR}/${output})
	add_custom_command(
		COMMAND
			${DXC_EXECUTABLE}
			-T ${profile}
			-Fo ${shader_output}
			-spirv
			${shader_src}
		DEPENDS ${shader_src}
		OUTPUT ${shader_output}
		COMMENT "Compiling ${shader_src}"
	)
	set(shader_outputs ${shader_outputs} ${shader_output} PARENT_SCOPE)
endfunction()

# Define shaders
orion_add_shader(shaders/vertex.hlsl vertex.spv vs_6_0)
orion_add_shader(shaders/fragment.hlsl fragment.spv ps_6_0)

# Create target for shaders
add_custom_target(shaders DEPENDS ${shader_outputs})
add_dependencies(orion shaders)
