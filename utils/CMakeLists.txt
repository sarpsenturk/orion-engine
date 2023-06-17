cmake_minimum_required(VERSION 3.21)

# Build/configuration options
option(ORION_UTILS_DEVELOPER_MODE "Enable developer mode" ON)
option(ORION_UTILS_TEST "Build tests for orion::utils" ${ORION_UTILS_DEVELOPER_MODE})
option(ORION_UTILS_INSTALL "Create install target for orion::utils" ${ORION_UTILS_DEVELOPER_MODE})

if (ORION_UTILS_TEST)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif ()


project(orion-utils
        VERSION 0.1.0
        DESCRIPTION "Common utilities for the orion game engine"
        LANGUAGES CXX)

# Find dependencies
find_package(fmt REQUIRED)

include(GNUInstallDirs)

set(ORION_UTILS_TOP_LEVEL ${PROJECT_IS_TOP_LEVEL})

# Declare the library
add_library(orion-utils INTERFACE "")
add_library(orion::utils ALIAS orion-utils)

# Set compile features
target_compile_features(orion-utils INTERFACE cxx_std_20)

# Set include directories
set(orion_utils_include_dirs
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_include_directories(orion-utils INTERFACE ${orion_utils_include_dirs})

# Link with dependencies
target_link_libraries(orion-utils INTERFACE fmt::fmt)

# Set export name for consistency between import and build targets
set_target_properties(orion-utils PROPERTIES EXPORT_NAME utils LINKER_LANGUAGE CXX)

# Create file set for public headers
target_sources(
        orion-utils PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS ${orion_utils_include_dirs}
)

# Add sources
add_subdirectory(include/orion-utils)

# Enable/disable testing
if (ORION_UTILS_TEST)
    enable_testing()
    add_subdirectory(tests)
endif ()

# Create install target
if (ORION_UTILS_INSTALL)
    include(CMakePackageConfigHelpers)
    set(export_target_name orion-utils-targets)
    set(cmake_dir ${CMAKE_INSTALL_LIBDIR}/cmake/orion-utils)
    set(config_file ${CMAKE_CURRENT_BINARY_DIR}/OrionUtilsConfig.cmake)
    set(version_file ${CMAKE_CURRENT_BINARY_DIR}/OrionUtilsConfigVersion.cmake)

    configure_package_config_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in ${config_file}
            INSTALL_DESTINATION ${cmake_dir}
    )
    write_basic_package_version_file(${version_file} COMPATIBILITY SameMajorVersion)

    install(
            TARGETS orion-utils
            EXPORT ${export_target_name}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILE_SET public_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT ${export_target_name} FILE ${export_target_name}.cmake NAMESPACE orion:: DESTINATION ${cmake_dir})
endif ()
