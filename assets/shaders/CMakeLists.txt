option(ORION_COMPILE_SPIRV "Compile Orion shaders to SPIR-V" ON)
option(ORION_COMPILE_DXIL "Compile Orion shaders to DXIL" ON)

function(orion_compile_shader filename il_target shader_output)
    get_filename_component(shader_name ${filename} NAME_WE)

    get_filename_component(shader_info ${filename} EXT)
    string(REPLACE "." ";" shader_info_list ${shader_info})

    list(GET shader_info_list 1 shader_type)
    set(shader_profile "${shader_type}_${ORION_SHADER_MODEL}")
    set(entry_point "${shader_type}_main")

    set(output ${ORION_SHADER_BIN_DIR}/${il_target}/${shader_name}.${shader_type})

    add_custom_command(
            OUTPUT ${output}
            COMMAND orion-shader-compiler -o ${output} -e ${entry_point} -t ${shader_profile} --${il_target} ${filename}
            WORKING_DIRECTORY ${ORION_SHADER_SOURCE_DIR}
            DEPENDS ${filename}
            VERBATIM
    )
    set(${shader_output} ${output} PARENT_SCOPE)
endfunction()

set(orion_shader_sources
        imgui.vs.hlsl imgui.ps.hlsl
        triangle.vs.hlsl triangle.ps.hlsl
        present.vs.hlsl present.ps.hlsl
        quad.vs.hlsl quad.ps.hlsl
)

set(orion_spirv_binaries "")
if (ORION_COMPILE_SPIRV)
    foreach (source ${orion_shader_sources})
        orion_compile_shader(${source} spirv shader_output)
        list(APPEND orion_spirv_binaries ${shader_output})
    endforeach ()
endif ()

set(orion_dxil_binaries "")
if (ORION_COMPILE_DXIL)
    foreach (source ${orion_shader_sources})
        orion_compile_shader(${source} dxil shader_output)
        list(APPEND orion_dxil_binaries ${shader_output})
    endforeach ()
endif ()

add_custom_target(orion-spirv-shaders DEPENDS ${orion_spirv_binaries})
add_custom_target(orion-dxil-shaders DEPENDS ${orion_dxil_binaries})
add_custom_target(orion-shaders DEPENDS orion-spirv-shaders orion-dxil-shaders)