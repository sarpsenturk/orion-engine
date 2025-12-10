target_sources(
	orion
	PUBLIC FILE_SET HEADERS FILES
	orion/orion.hpp
	orion/application.hpp
	orion/log.hpp
	orion/assert.hpp

	orion/platform/platform.hpp
	orion/platform/window.hpp
	orion/platform/input.hpp
	orion/platform/time.hpp

	orion/math/vector.hpp
)
