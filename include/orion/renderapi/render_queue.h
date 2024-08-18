#pragma once

namespace orion
{
    class CommandQueue
    {
    public:
        CommandQueue() = default;
        virtual ~CommandQueue() = default;

    protected:
        CommandQueue(CommandQueue const&) = default;
        CommandQueue& operator=(CommandQueue const&) = default;
        CommandQueue(CommandQueue&&) = default;
        CommandQueue& operator=(CommandQueue&&) = default;
    };
} // namespace orion
