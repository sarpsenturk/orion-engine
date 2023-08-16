# Module specific options
option(ORION_CORE_TEST "Build tests for orion-core" ${ORION_ENGINE_DEVELOPER_MODE})

# Module specific dependencies
find_package(spdlog REQUIRED)

# Create core library
add_library(orion-core "")

# Create alias for consistency
add_library(orion::core ALIAS orion-core)

# Set include directories
target_include_directories(orion-core PUBLIC include/)

# Link with dependencies
target_link_libraries(
        orion-core 
        PUBLIC
        spdlog::spdlog
        orion::utils
        orion::math
)

# Precompiled headers
target_precompile_headers(orion-core REUSE_FROM orion-pch)

# Set compile time flags for spdlog
target_compile_definitions(
        orion-core PUBLIC
        $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>
        $<$<CONFIG:RelWithDebInfo>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
        $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_WARN>
)

# Create file set for public headers
target_sources(orion-core PUBLIC FILE_SET public_headers TYPE HEADERS)

# Add sources
add_subdirectory(include/orion-core)
add_subdirectory(src)

# Add configuration file
configure_file(config.h.in ${CMAKE_BINARY_DIR}/include/orion-core/config.h)

# Enable/disable tests
if (ORION_CORE_TEST AND ORION_ENGINE_TEST)
    add_subdirectory(tests)
endif ()
