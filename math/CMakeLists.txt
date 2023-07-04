option(ORION_MATH_TEST "Build tests for orion-math" ${ORION_ENGINE_TEST})

# Dependencies
find_package(fmt REQUIRED)

# Application library
add_library(orion-math INTERFACE "")
target_compile_features(orion-math INTERFACE cxx_std_20)
target_include_directories(orion-math INTERFACE include/)
target_link_libraries(orion-math INTERFACE fmt::fmt orion::utils)

# Create file set
target_sources(orion-math INTERFACE FILE_SET public_headers TYPE HEADERS)

# Add sources
add_subdirectory(include/orion-math)

# Create alias
add_library(orion::math ALIAS orion-math)

# Enable/disable tests
if (ORION_MATH_TEST AND ORION_ENGINE_TEST)
    add_subdirectory(tests)
endif ()
