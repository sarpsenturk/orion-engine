add_executable(sandbox sandbox.cpp)
target_link_libraries(sandbox PRIVATE orion)
set_target_properties(sandbox PROPERTIES OUTPUT_NAME "orion-sandbox")

install(TARGETS sandbox EXPORT OrionTargets)