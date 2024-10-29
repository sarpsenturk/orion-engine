target_sources(
        orion PUBLIC
        FILE_SET HEADERS FILES
        orion/application.hpp
        orion/assertion.hpp
        orion/compiler.hpp
        orion/input.hpp
        orion/platform.hpp
        orion/window.hpp

        orion/math/sqrt.hpp

        orion/math/matrix/matrix.hpp
        orion/math/matrix/matrix2.hpp
        orion/math/matrix/matrix3.hpp
        orion/math/matrix/matrix4.hpp
        orion/math/matrix/projection.hpp
        orion/math/matrix/transformation.hpp

        orion/math/vector/vector.hpp
        orion/math/vector/vector2.hpp
        orion/math/vector/vector3.hpp
        orion/math/vector/vector4.hpp

        orion/renderapi/buffer.hpp
        orion/renderapi/descriptor.hpp
        orion/renderapi/format.hpp
        orion/renderapi/handle.hpp
        orion/renderapi/image.hpp
        orion/renderapi/pipeline.hpp
        orion/renderapi/render_backend.hpp
        orion/renderapi/render_command.hpp
        orion/renderapi/render_device.hpp
        orion/renderapi/render_queue.hpp
        orion/renderapi/shader.hpp
        orion/renderapi/swapchain.hpp
        orion/renderapi/sync.hpp

        orion/renderer/camera.hpp
        orion/renderer/color.hpp
        orion/renderer/renderer.hpp
        orion/renderer/sprite_renderer.hpp
)
