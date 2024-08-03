add_executable(orion-sandbox sandbox.cpp)
target_link_libraries(orion-sandbox orion::orion)
install(TARGETS orion-sandbox EXPORT OrionTargets)
