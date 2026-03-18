target_sources(
    orion
    PRIVATE
    orion.cpp
    application.cpp
    log.cpp
    debug.cpp
    window.cpp
    event.cpp

    platform/platform_glfw.cpp
)

