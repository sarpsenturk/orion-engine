
add_executable(orion-sandbox main.cpp)
target_link_libraries(orion-sandbox orion::engine)
set_target_properties(orion-sandbox PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
