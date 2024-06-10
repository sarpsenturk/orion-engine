#pragma once

#include "orion-renderapi/handles.h"

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

    struct SubmitDesc {
        CommandQueueType queue_type;
        std::span<const SemaphoreHandle> wait_semaphores;
        std::span<const CommandList* const> command_lists;
        std::span<const SemaphoreHandle> signal_semaphores;
    };

    struct FenceDesc {
        bool start_finished;
    };
} // namespace orion
