option(ORION_PLATFORM_TEST "Build tests for orion-platform" ${ORION_DEVELOPER_MODE})

find_package(spdlog CONFIG REQUIRED)

add_library(orion-platform "")
add_library(orion::platform ALIAS orion-platform)

target_include_directories(orion-platform PUBLIC include/)

target_link_libraries(
        orion-platform
        PUBLIC
        orion::math
        PRIVATE
        spdlog::spdlog
        orion::utils
)

target_sources(orion-platform PUBLIC FILE_SET public_headers TYPE HEADERS)

add_subdirectory(include/orion-platform)
add_subdirectory(src)

if (ORION_PLATFORM_WINDOWS)
    add_subdirectory(win32/include/orion-win32)
    add_subdirectory(win32/src)
    target_include_directories(orion-platform PUBLIC win32/include)
else ()
    message(FATAL_ERROR "orion is not implemented on this platform")
endif ()
