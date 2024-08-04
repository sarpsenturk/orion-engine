cmake_minimum_required(VERSION 3.23)

# Set project version
set(ORION_VERSION_MAJOR "0" CACHE STRING "" FORCE)
set(ORION_VERSION_MINOR "1" CACHE STRING "" FORCE)
set(ORION_VERSION_PATCH "0" CACHE STRING "" FORCE)
set(ORION_VERSION "${ORION_VERSION_MAJOR}.${ORION_VERSION_MINOR}.${ORION_VERSION_PATCH}" CACHE STRING "" FORCE)

# Create project
project(
        orion-engine
        VERSION ${ORION_VERSION}
        LANGUAGES CXX
)

include(GNUInstallDirs)
set(orion_includedir ${CMAKE_INSTALL_INCLUDEDIR})

# Create orion library
add_library(orion "")
add_library(orion::orion ALIAS orion)
target_compile_features(orion PUBLIC cxx_std_20)
target_compile_definitions(orion PUBLIC $<$<CONFIG:Debug>:ORION_BUILD_DEBUG>)

# Set include directories
target_include_directories(orion PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/conf>
        $<INSTALL_INTERFACE:${orion_includedir}>
)

# Set compile options for compiler
include(cmake/CompileOptions.cmake)

# Add dependencies
find_package(fmt 11.0.0 CONFIG REQUIRED)
find_package(tl-expected 1.0.0 CONFIG REQUIRED)
target_link_libraries(orion PUBLIC fmt::fmt tl::expected)

# Add sources
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(examples)

# Add entry point
add_library(orion-main src/entry_point.cpp)
add_library(orion::orion-main ALIAS orion-main)
target_link_libraries(orion-main PRIVATE orion)
target_compile_features(orion-main PUBLIC cxx_std_20)

# Add & enable tests
if (PROJECT_IS_TOP_LEVEL)
    include(CTest)
    if (BUILD_TESTING)
        add_subdirectory(tests)
    endif ()
endif ()

# Set install directories
set(orion_bindir ${CMAKE_INSTALL_BINDIR})
set(orion_libdir ${CMAKE_INSTALL_LIBDIR}/orion-${ORION_VERSION})
set(orion_cmakedir ${CMAKE_INSTALL_LIBDIR}/orion-${ORION_VERSION}/cmake)

# Create version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        OrionConfigVersion.cmake
        VERSION ${ORION_VERSION}
        COMPATIBILITY SameMajorVersion
)
install(
        FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/orion-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/OrionConfigVersion.cmake
        DESTINATION
        ${orion_cmakedir}
)

# Install orion libs, headers
install(
        TARGETS orion orion-main
        EXPORT OrionTargets
        LIBRARY DESTINATION ${orion_libdir}
        ARCHIVE DESTINATION ${orion_libdir}
        RUNTIME DESTINATION ${orion_bindir}
        FILE_SET HEADERS DESTINATION ${orion_includedir}
)

# Install configuration file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/conf/orion/config.h DESTINATION ${orion_includedir}/orion)

# Install export target
install(
        EXPORT OrionTargets
        NAMESPACE orion::
        DESTINATION ${orion_cmakedir}
)
