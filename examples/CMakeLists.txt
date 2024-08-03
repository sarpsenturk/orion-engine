add_executable(orion-sandbox sandbox.cpp)
target_link_libraries(orion-sandbox orion::orion orion::orion-main)
install(TARGETS orion-sandbox EXPORT OrionTargets)
