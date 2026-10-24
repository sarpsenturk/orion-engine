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
)

# Third-party libraries
target_link_libraries(orion.engine
	PUBLIC
		spdlog::spdlog
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
		"orion/core/window.hpp"
		"orion/core/event.hpp"
		"orion/core/input.hpp"

		"orion/utils/array.hpp"
		"orion/utils/scope_guard.hpp"
		"orion/utils/byte_span.hpp"

		"orion/renderer/renderer.hpp"
		"orion/gpu/gpu.hpp"
	PRIVATE
		"orion/orion.cpp"

		"orion/core/logger.cpp"
		"orion/core/assert.cpp"
		"orion/core/platform.hpp"
		"orion/core/platform_impl_glfw.cpp"
		"orion/core/event.cpp"
		"orion/core/input.cpp"

		"orion/renderer/renderer.cpp"
		"orion/gpu/gpu_impl_vulkan.cpp"
)

# Engine shaders
# Offline compilation using slangc
add_subdirectory(shaders)
add_dependencies(orion.engine orion.shaders)
