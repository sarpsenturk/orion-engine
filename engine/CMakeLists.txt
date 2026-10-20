# Engine configuration options
set(
	ORION_SHADER_OUTPUT_DIR
	"${CMAKE_CURRENT_BINARY_DIR}/shaders" 
	CACHE PATH
	"Directory for compiled shader (SPIR-V) output"
)

# Engine core library
add_library(orion.engine "")

# C++ standard
target_compile_features(orion.engine PUBLIC cxx_std_23)

# Compiler warnings
# Project is built from day one with warnings as errors.
target_compile_options(orion.engine
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /external:anglebrackets /external:W0 /permissive->
)

# Preprocessor definitions
target_compile_definitions(orion.engine
	PUBLIC
		$<$<CONFIG:Debug>:ORION_DEBUG>
	PRIVATE
		ORION_SHADER_OUTPUT_DIR="${ORION_SHADER_OUTPUT_DIR}"
)

# Third-party libraries
add_subdirectory(imgui)
target_link_libraries(orion.engine
	PUBLIC
		spdlog::spdlog
		glm::glm
		imgui
	PRIVATE
		glfw
		Vulkan::Headers Vulkan::volk
)

# Engine sources
# - Public headers are listed under FILE_SET HEADERS
target_sources(orion.engine
	PUBLIC FILE_SET HEADERS FILES
		"orion/orion.hpp"

		"orion/core/logger.hpp"
		"orion/core/assert.hpp"
		"orion/core/platform.hpp"
		"orion/core/window.hpp"
		"orion/core/input.hpp"

		"orion/memory/memory.hpp"
		"orion/memory/offset_allocator.hpp"

		"orion/renderer/renderer.hpp"
		"orion/renderer/imgui_renderer.hpp"
		"orion/renderer/geometry.hpp"
		"orion/renderer/render_graph/builder.hpp"
		"orion/renderer/render_graph/context.hpp"
		"orion/renderer/render_graph/graph.hpp"
		"orion/renderer/render_graph/resource.hpp"
		"orion/renderer/gpu/gpu.hpp"
		"orion/renderer/gpu/deletion_queue.hpp"

		"orion/lib/scope_guard.hpp"
		"orion/lib/array.hpp"
		"orion/lib/slot_map.hpp"
		"orion/lib/hash.hpp"
		"orion/lib/inline_function.hpp"

		"orion/controllers/fly_camera.hpp"
	PRIVATE
		"orion/orion.cpp"

		"orion/core/logger.cpp"
		"orion/core/assert.cpp"
		"orion/core/platform_impl_glfw.cpp"

		"orion/memory/offset_allocator.cpp"

		"orion/renderer/renderer.cpp"
		"orion/renderer/imgui_renderer.cpp"
		"orion/renderer/geometry.cpp"
		"orion/renderer/render_graph/builder.cpp"
		"orion/renderer/render_graph/context.cpp"
		"orion/renderer/render_graph/graph.cpp"
		"orion/renderer/render_graph/resource.cpp"
		"orion/renderer/gpu/gpu_impl_vulkan.cpp"
		"orion/renderer/gpu/deletion_queue.cpp"

		"orion/controllers/fly_camera.cpp"
)

# Engine shaders
# Offline compilation using slangc
add_subdirectory(shaders)
add_dependencies(orion.engine orion.shaders)

# Engine tests
if (ORION_BUILD_TESTS)
	add_subdirectory(tests)
endif()
