cmake_minimum_required(VERSION 3.25)

# Set project version
set(ORION_VERSION_MAJOR 0 CACHE INTERNAL "Orion major version")
set(ORION_VERSION_MINOR 1 CACHE INTERNAL "Orion minor version")
set(ORION_VERSION_PATCH 0 CACHE INTERNAL "Orion patch version")
set(ORION_VERSION "${ORION_VERSION_MAJOR}.${ORION_VERSION_MINOR}.${ORION_VERSION_PATCH}" CACHE INTERNAL "Orion version")

# Define project
project(
	orion
	VERSION ${ORION_VERSION}
	LANGUAGES C CXX
)

include(GNUInstallDirs)

# Find dependencies
find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

# Create library target
add_library(orion "")
target_compile_features(orion PUBLIC cxx_std_20)
target_include_directories(
	orion
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_options(orion PRIVATE ${ORION_CXX_FLAGS}) # Set in CMakePresets.json
target_compile_definitions(
	orion
	PRIVATE
	$<$<CONFIG:Debug>:ORION_ENABLE_ASSERTIONS=1>
)
target_link_libraries(
	orion
	PUBLIC
	spdlog::spdlog
	PRIVATE
	glfw
)

# Add library sources
add_subdirectory(src)
add_subdirectory(include)

# Add example executables
add_subdirectory(examples)

# Add tests
include(CTest)
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Install targets
install(
	TARGETS orion
	EXPORT OrionTargets
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets
install(
	EXPORT OrionTargets
	FILE OrionTargets.cmake
	NAMESPACE orion::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/orion
)

# Create & install config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/OrionConfigVersion.cmake
    VERSION ${ORION_VERSION}
    COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/OrionConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/OrionConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/orion
)
install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/OrionConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/OrionConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/orion
)
