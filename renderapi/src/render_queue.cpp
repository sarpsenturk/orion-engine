#include "orion-renderapi/render_queue.h"

namespace orion
{
    void CommandQueue::wait(SemaphoreHandle semaphore)
    {
        wait_api(semaphore);
    }

    void CommandQueue::signal(SemaphoreHandle semaphore)
    {
        signal_api(semaphore);
    }

    void CommandQueue::submit(std::span<const CommandList* const> command_lists, FenceHandle signal_fence)
    {
        submit_api(command_lists, signal_fence);
    }

    void CommandQueue::submit(const CommandList* command_list, FenceHandle signal_fence)
    {
        submit_api({{command_list}}, signal_fence);
    }

    void CommandQueue::submit_immediate(std::span<const CommandList* const> command_lists)
    {
        submit_immediate_api(command_lists);
    }

    void CommandQueue::submit_immediate(const CommandList* command_list)
    {
        submit_immediate_api({{command_list}});
    }
} // namespace orion
