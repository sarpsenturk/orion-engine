option(ORION_UTILS_TEST "Enable tests for orion::utils" ${ORION_ENGINE_TEST})

if (ORION_UTILS_TEST)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif ()

# Find dependencies
find_package(fmt REQUIRED)

# Declare the library
add_library(orion-utils INTERFACE "")
add_library(orion::utils ALIAS orion-utils)

# Set compile features
target_compile_features(orion-utils INTERFACE cxx_std_20)

# Set include directories
target_include_directories(orion-utils INTERFACE include/)

# Link with dependencies
target_link_libraries(orion-utils INTERFACE fmt::fmt)

# Create file set for public headers
target_sources(orion-utils INTERFACE FILE_SET public_headers TYPE HEADERS)

# Add sources
add_subdirectory(include/orion-utils)

# Enable/disable testing
if (ORION_UTILS_TEST)
    add_subdirectory(tests)
endif ()
