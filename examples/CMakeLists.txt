find_package(fastgltf 0.9.1 CONFIG REQUIRED)

add_executable(orion.sandbox "sandbox/main.cpp")
target_link_libraries(orion.sandbox PRIVATE orion.engine fastgltf::fastgltf)
