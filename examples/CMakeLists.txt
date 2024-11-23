add_executable(orion-sandbox sandbox.cpp)
target_link_libraries(orion-sandbox orion::orion orion::orion-main)

add_executable(orion-pong pong.cpp)
target_link_libraries(orion-pong orion::orion orion::orion-main)

install(TARGETS orion-sandbox orion-pong EXPORT OrionTargets)
