#pragma once

#include "orion-renderapi/handles.h"

#include <span>

namespace orion
{
    enum class CommandQueueType {
        Graphics,
        Transfer,
        Compute,
        Any
    };

    // Forward declare
    class CommandList;

    struct FenceDesc {
        bool start_finished;
    };

    class CommandList;

    class CommandQueue
    {
    public:
        CommandQueue() = default;
        virtual ~CommandQueue() = default;

        void wait(SemaphoreHandle semaphore);
        void signal(SemaphoreHandle semaphore);
        void submit(std::span<const CommandList* const> command_lists, FenceHandle signal_fence);
        void submit(const CommandList* command_list, FenceHandle signal_fence);
        void submit_immediate(std::span<const CommandList* const> command_lists);
        void submit_immediate(const CommandList* command_list);

    protected:
        CommandQueue(const CommandQueue&) = default;
        CommandQueue(CommandQueue&&) = default;
        CommandQueue& operator=(const CommandQueue&) = default;
        CommandQueue& operator=(CommandQueue&&) = default;

    private:
        virtual void wait_api(SemaphoreHandle semaphore) = 0;
        virtual void signal_api(SemaphoreHandle semaphore) = 0;
        virtual void submit_api(std::span<const CommandList* const> command_lists, FenceHandle signal_fence) = 0;
        virtual void submit_immediate_api(std::span<const CommandList* const> command_lists) = 0;
    };
} // namespace orion
