cmake_minimum_required(VERSION 3.12)

project(orion-engine
        DESCRIPTION "A cross platform game engine written in C++ 20"
        VERSION 0.1.0
        LANGUAGES CXX)

# External dependencies
add_subdirectory(external/spdlog)
add_subdirectory(external/arg-parse)
add_subdirectory(external/orion-math)

if(WIN32)
    set(ORION_PLATFORM_SOURCES
            include/orion/platform/win32/win_include.h
            include/orion/platform/win32/win_window.h

            src/platform/win32/win_window.cpp
            )
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/orion.config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/include/orion/config.h")

add_library(orion-engine STATIC
        # Public headers
        include/orion/orion.h

        include/orion/core/engine.h
        include/orion/core/application.h
        include/orion/core/entry_point.h

        include/orion/window/window.h
        include/orion/window/window_props.h

        include/orion/events/event_handler.h
        include/orion/events/event_dispatcher.h
        include/orion/events/window_event.h
        include/orion/events/mouse_event.h

        include/orion/input/mouse.h

        include/orion/utility/index_of.h
        include/orion/utility/contains.h

        # Source files
        src/core/engine.cpp
        src/core/application.cpp

        # Platform specific sources
        ${ORION_PLATFORM_SOURCES}
        )

target_compile_features(orion-engine PUBLIC cxx_std_20)
target_link_libraries(orion-engine
        PUBLIC
        spdlog arg-parse orion-math
        )
target_include_directories(orion-engine PUBLIC include/)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} /orion/orion.h

        include/orion/core/engine.h
        include/orion/core/application.h
        include/orion/core/entry_point.h

        include/orion/window/window.h
        include/orion/window/window_props.h

        include/orion/events/event_handler.h
        include/orion/events/event_dispatcher.h
        include/orion/events/window_event.h

        include/orion/utility/index_of.h

        # Source files
        src/core/engine.cpp
        src/core/application.cpp

        # Platform specific sources
        ${ORION_PLATFORM_SOURCES}
        )

target_compile_features(orion-engine PUBLIC cxx_std_20)
target_link_libraries(orion-engine
        PUBLIC
        spdlog arg-parse orion-math
        )
target_include_directories(orion-engine PUBLIC include/)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(EnableGoogleTest)
EnableGoogleTest(ORION_ENGINE)

option(ORION_BUILD_SANDBOX "Build the Orion sandbox project" OFF)
if (ORION_BUILD_SANDBOX)
    add_subdirectory(Sandbox)
endif ()
