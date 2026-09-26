# Engine shaders target
add_custom_target(orion.shaders)

# Helper function to declare a new slang shader program.
function(orion_add_shader shader_source)
	get_filename_component(shader_name ${shader_source} NAME_WE)
	set(shader_output "${ORION_SHADER_OUTPUT_DIR}/${shader_name}.spv")
	set(shader_depfile "${shader_output}.d")
	add_custom_command(
		OUTPUT ${shader_output}
		COMMAND
			${slangc_EXE}
			"${CMAKE_CURRENT_SOURCE_DIR}/${shader_source}"
			-target spirv
			-profile spirv_1_6
			-I ${CMAKE_CURRENT_SOURCE_DIR}
			-depfile ${shader_depfile}
			-o ${shader_output}
		DEPFILE ${shader_depfile}
		DEPENDS ${shader_source}
		COMMENT "Building SPV object ${shader_output}"
		VERBATIM
	)
	target_sources(orion.shaders PRIVATE ${shader_output})
	set_source_files_properties(${shader_output} PROPERTIES GENERATED TRUE)
endfunction()

# Declare shader programs
orion_add_shader("triangle.slang")
