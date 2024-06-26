
# Set asset and sub-asset directories
set(ORION_ASSET_DIR assets)
set(ORION_SHADER_DIR ${ORION_ASSET_DIR}/shaders)
set(ORION_EFFECT_DIR ${ORION_ASSET_DIR}/effects)
set(ORION_TEXTURE_DIR ${ORION_ASSET_DIR}/textures)

# Set asset source directories
set(ORION_ASSET_SOURCE_DIR ${CMAKE_SOURCE_DIR}/${ORION_ASSET_DIR})
set(ORION_SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/${ORION_SHADER_DIR})
set(ORION_EFFECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/${ORION_EFFECT_DIR})
set(ORION_TEXTURE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/${ORION_TEXTURE_DIR})

# Set asset binary directories
set(ORION_ASSET_BIN_DIR ${CMAKE_BINARY_DIR}/bin/${ORION_ASSET_DIR})
set(ORION_SHADER_BIN_DIR ${CMAKE_BINARY_DIR}/bin/${ORION_SHADER_DIR})
set(ORION_EFFECT_BIN_DIR ${CMAKE_BINARY_DIR}/bin/${ORION_EFFECT_DIR})
set(ORION_TEXTURE_BIN_DIR ${CMAKE_BINARY_DIR}/bin/${ORION_TEXTURE_DIR})

# Set dxil & spirv directories
set(ORION_SPIRV_DIR ${ORION_SHADER_DIR}/spirv)
set(ORION_DXIL_DIR ${ORION_SHADER_DIR}/dxil)

# Shader model configuration
set(ORION_SHADER_MODEL "6_0" CACHE STRING "HLSL shader model to use")

# Set shader profile names
set(ORION_VS_PROFILE "vs_${ORION_SHADER_MODEL}")
set(ORION_PS_PROFILE "ps_${ORION_SHADER_MODEL}")
set(ORION_CS_PROFILE "cs_${ORION_SHADER_MODEL}")
set(ORION_GS_PROFILE "gs_${ORION_SHADER_MODEL}")
set(ORION_DS_PROFILE "ds_${ORION_SHADER_MODEL}")
set(ORION_HS_PROFILE "hs_${ORION_SHADER_MODEL}")

# Set profile entry points
set(ORION_VS_ENTRY "vs_main")
set(ORION_PS_ENTRY "ps_main")
set(ORION_CS_ENTRY "cs_main")
set(ORION_GS_ENTRY "gs_main")
set(ORION_DS_ENTRY "ds_main")
set(ORION_HS_ENTRY "hs_main")

# Compile shaders
add_subdirectory(shaders)

# TODO: Come up with a better asset system for CMake so we don't copy files every build

# Create effect target
add_custom_target(orion-effects COMMAND ${CMAKE_COMMAND} -E copy_directory ${ORION_EFFECT_SOURCE_DIR} ${ORION_EFFECT_BIN_DIR})

# Create texture target
add_custom_target(orion-textures COMMAND ${CMAKE_COMMAND} -E copy_directory ${ORION_TEXTURE_SOURCE_DIR} ${ORION_TEXTURE_BIN_DIR})

# Add configuration file
configure_file(config.h.in ${CMAKE_BINARY_DIR}/include/orion-assets/config.h)

# Add assets target
add_custom_target(orion-assets DEPENDS orion-shaders orion-effects orion-textures)
