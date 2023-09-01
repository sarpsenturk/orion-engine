option(ORION_SCENE_TEST "Enable tests for orion-scene" ${ORION_ENGINE_DEVELOPER_MODE})

find_package(EnTT CONFIG REQUIRED)

add_library(orion-scene "")
add_library(orion::scene ALIAS orion-scene)

target_link_libraries(
        orion-scene
        PUBLIC
        orion::core
        orion::utils
        orion::math
        EnTT::EnTT
)

target_include_directories(orion-scene PUBLIC include/)

target_precompile_headers(orion-scene REUSE_FROM orion-pch)

target_sources(orion-scene PUBLIC FILE_SET public_headers TYPE HEADERS)

add_subdirectory(include/orion-scene)
add_subdirectory(src)

if (ORION_SCENE_TEST)
    add_subdirectory(tests)
endif ()
