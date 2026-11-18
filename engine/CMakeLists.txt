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
target_link_libraries(orion.engine
	PUBLIC
		glm::glm
		imgui
		EnTT::EnTT
	PRIVATE
		spdlog::spdlog
		glfw
		Vulkan::Headers Vulkan::volk
)

# Engine sources
# - Public headers are listed under FILE_SET HEADERS
target_sources(orion.engine
	PUBLIC FILE_SET HEADERS FILES
		"orion/orion.hpp"
		"orion/app.hpp"
		"orion/world.hpp"

		"orion/core/assert.hpp"
		"orion/core/logger.hpp"
		"orion/core/window.hpp"
		"orion/core/event.hpp"
		"orion/core/input.hpp"

		"orion/utils/array.hpp"
		"orion/utils/scope_guard.hpp"
		"orion/utils/byte_span.hpp"
		"orion/utils/slot_map.hpp"
		"orion/utils/inplace_function.hpp"
		"orion/utils/timer.hpp"

		"orion/gpu/gpu.hpp"
		"orion/gpu/range_allocator.hpp"
		"orion/gpu/page_allocator.hpp"
		"orion/gpu/deletion_queue.hpp"

		"orion/loaders/texture.hpp"
	PRIVATE
		"orion/orion.cpp"
		"orion/app.cpp"

		"orion/core/logger.cpp"
		"orion/core/assert.cpp"
		"orion/core/logger.cpp"
		"orion/core/window.cpp"
		"orion/core/event.cpp"
		"orion/core/input.cpp"

		"orion/gpu/gpu_impl_vulkan.cpp"
		"orion/gpu/deletion_queue.cpp"
		"orion/gpu/page_allocator.cpp"

		"orion/loaders/stb_image.h"
		"orion/loaders/texture.cpp"
)

# Engine shaders
# Offline compilation using slangc
add_subdirectory(shaders)
add_dependencies(orion.engine orion.shaders)

# Engine tests
if (ORION_BUILD_TESTS)
	add_subdirectory(tests)
endif()
