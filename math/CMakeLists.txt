cmake_minimum_required(VERSION 3.23)

project(OrionMath
        VERSION 0.1.0
        DESCRIPTION "Math library for the Orion game engine"
        LANGUAGES CXX)

include(GNUInstallDirs)

option(ORION_MATH_TEST "Generate test targets orion-math" ${PROJECT_IS_TOP_LEVEL})
option(ORION_MATH_INSTALL "Generate install target" ON)

# Add our CMake modules
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Dependencies
find_package(fmt REQUIRED)

# Application library
add_library(orion_math INTERFACE "")
target_compile_features(orion_math INTERFACE cxx_std_20)
target_include_directories(orion_math INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )
target_link_libraries(orion_math INTERFACE fmt::fmt)

# Create file set
target_sources(
        orion_math
        INTERFACE
        FILE_SET orion_math_headers
        TYPE HEADERS
        BASE_DIRS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Add sources
add_subdirectory(include/orion-math)

# Create alias
add_library(orion::math ALIAS orion_math)

# Enable/disable tests
if (ORION_MATH_TEST)
    enable_testing()
    find_package(GTest REQUIRED)
    add_subdirectory(tests)
endif ()

# Install targets
if (ORION_MATH_INSTALL)
    include(CMakePackageConfigHelpers)

    set(version_config ${PROJECT_BINARY_DIR}/orion-math-config-version.cmake)
    set(project_config ${PROJECT_BINARY_DIR}/orion-math-config.cmake)
    set(install_targets orion_math)
    set(target_exports orion-math-targets)
    set(cmake_dir ${CMAKE_INSTALL_LIBDIR}/cmake/orion-math)

    # Create version config file
    write_basic_package_version_file(
            ${version_config}
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
    )

    install(
            TARGETS ${install_targets}
            EXPORT ${target_exports}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            FILE_SET orion_math_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    configure_package_config_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/orion-math-config.in
            ${project_config}
            INSTALL_DESTINATION ${cmake_dir}
    )

    install(
            EXPORT ${target_exports}
            NAMESPACE orion::
            DESTINATION ${cmake_dir})
    install(FILES ${project_config} ${version_config}
            DESTINATION ${cmake_dir})
endif ()
