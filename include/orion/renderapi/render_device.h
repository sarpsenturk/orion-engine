#pragma once

#include "orion/renderapi/render_queue.h"

#include <memory>

namespace orion
{
    class RenderDevice
    {
    public:
        RenderDevice() = default;
        virtual ~RenderDevice() = default;

        std::unique_ptr<CommandQueue> create_command_queue();

    protected:
        RenderDevice(const RenderDevice&) = default;
        RenderDevice& operator=(const RenderDevice&) = default;
        RenderDevice(RenderDevice&&) = default;
        RenderDevice& operator=(RenderDevice&&) = default;

    private:
        virtual std::unique_ptr<CommandQueue> create_command_queue_api() = 0;
    };
} // namespace orion
