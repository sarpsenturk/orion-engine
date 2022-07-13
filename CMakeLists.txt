cmake_minimum_required(VERSION 3.12)

project(orion-engine
        DESCRIPTION "A cross platform game engine written in C++ 20"
        VERSION 0.1.0
        LANGUAGES CXX)

add_library(orion-engine STATIC
        # Public headers
        include/orion/orion.h

        # Source files
        src/orion.cpp
        )

target_compile_features(orion-engine PUBLIC cxx_std_20)
target_include_directories(orion-engine PUBLIC include/)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(EnableGoogleTest)
EnableGoogleTest(ORION_ENGINE)
