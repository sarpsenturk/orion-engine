option(ORION_SHADER_COMPILER_TEST "Enable tests for the Orion Shader Compiler" ${ORION_DEVELOPER_MODE})

# Find dependencies
find_package(directx-dxc CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# Create shader compiler library
add_library(orion-shader-compiler-lib "")
add_library(orion::shader-compiler ALIAS orion-shader-compiler-lib)

# Set include directories
target_include_directories(orion-shader-compiler-lib PUBLIC include/)

# Link with dependencies
target_link_libraries(
        orion-shader-compiler-lib
        PUBLIC
        orion::utils
        PRIVATE
        Microsoft::DirectXShaderCompiler
        spdlog::spdlog
)

# Precompiled headers
target_precompile_headers(orion-shader-compiler-lib REUSE_FROM orion-pch)

# Create file set for public headers
target_sources(orion-shader-compiler-lib PUBLIC FILE_SET public_headers TYPE HEADERS)

# Add sources
add_subdirectory(include/orion-shader-compiler)
add_subdirectory(src)

# Enable/disable tests
if (ORION_SHADER_COMPILER_TEST AND ORION_TEST)
    add_subdirectory(tests)
endif ()

# Build the offline shader compiler executable
find_package(CLI11 CONFIG REQUIRED)
add_executable(orion-shader-compiler main.cpp)
target_link_libraries(orion-shader-compiler orion-shader-compiler-lib CLI11::CLI11)
