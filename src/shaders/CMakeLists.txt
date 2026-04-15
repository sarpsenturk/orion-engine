# Create custom shader target
add_custom_target(orion.shader)

# Adds a custom command to compile shader_src
function(orion_compile_shader shader_src)
	get_filename_component(shader_name ${shader_src} NAME)
	set(shader_output ${CMAKE_BINARY_DIR}/shaders/${shader_name}.spv)
	add_custom_command(
		OUTPUT ${shader_output}
		COMMAND Vulkan::glslc ${shader_src} -o ${shader_output}
		DEPENDS ${shader_src}
		COMMENT "glslc ${shader_src} -o ${shader_output}"
		VERBATIM
	)
	target_sources(orion.shader PRIVATE ${shader_output})
endfunction()

# Compile shaders
orion_compile_shader(${CMAKE_CURRENT_SOURCE_DIR}/triangle.vert)
orion_compile_shader(${CMAKE_CURRENT_SOURCE_DIR}/triangle.frag)

# Add dependency from orion to shader target
add_dependencies(orion orion.shader)
