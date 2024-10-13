#pragma once

#include "orion/renderapi/handle.hpp"

#include <span>

namespace orion
{
    class CommandQueue
    {
    public:
        CommandQueue() = default;
        virtual ~CommandQueue() = default;

        void wait(SemaphoreHandle semaphore);
        void signal(SemaphoreHandle semaphore);

        void submit(std::span<const class CommandList* const> command_lists, FenceHandle fence);

    protected:
        CommandQueue(CommandQueue const&) = default;
        CommandQueue& operator=(CommandQueue const&) = default;
        CommandQueue(CommandQueue&&) = default;
        CommandQueue& operator=(CommandQueue&&) = default;

    private:
        virtual void wait_api(SemaphoreHandle semaphore) = 0;
        virtual void signal_api(SemaphoreHandle semaphore) = 0;

        virtual void submit_api(std::span<const class CommandList* const> command_lists, FenceHandle fence) = 0;
    };
} // namespace orion
