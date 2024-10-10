target_sources(
        orion PUBLIC
        FILE_SET HEADERS FILES
        orion/application.h
        orion/assertion.h
        orion/compiler.h
        orion/platform.h
        orion/window.h

        orion/math/matrix/matrix.h
        orion/math/matrix/matrix2.h
        orion/math/matrix/matrix3.h
        orion/math/matrix/matrix4.h
        orion/math/matrix/projection.h

        orion/math/vector/vector.h
        orion/math/vector/vector2.h
        orion/math/vector/vector3.h
        orion/math/vector/vector4.h

        orion/renderapi/buffer.h
        orion/renderapi/descriptor.h
        orion/renderapi/format.h
        orion/renderapi/handle.h
        orion/renderapi/pipeline.h
        orion/renderapi/render_backend.h
        orion/renderapi/render_command.h
        orion/renderapi/render_device.h
        orion/renderapi/render_queue.h
        orion/renderapi/shader.h
        orion/renderapi/swapchain.h
        orion/renderapi/sync.h
)
