#include "orion/renderapi/render_queue.hpp"

#include "orion/assertion.hpp"

namespace orion
{
    void CommandQueue::wait(SemaphoreHandle semaphore)
    {
        ORION_ASSERT(semaphore != SemaphoreHandle::Invalid);
        wait_api(semaphore);
    }

    void CommandQueue::signal(SemaphoreHandle semaphore)
    {
        ORION_ASSERT(semaphore != SemaphoreHandle::Invalid);
        signal_api(semaphore);
    }

    void CommandQueue::submit(std::span<const class CommandList* const> command_lists, FenceHandle fence)
    {
        submit_api(command_lists, fence);
    }
} // namespace orion
