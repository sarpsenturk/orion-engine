#pragma once

namespace orion
{
    class CommandList
    {
    public:
        CommandList() = default;
        virtual ~CommandList() = default;

        void begin();
        void end();

    protected:
        CommandList(const CommandList&) = default;
        CommandList& operator=(const CommandList&) = default;
        CommandList(CommandList&&) = default;
        CommandList& operator=(CommandList&&) = default;

    private:
        virtual void begin_api() = 0;
        virtual void end_api() = 0;
    };

    class CommandAllocator
    {
    public:
        CommandAllocator() = default;
        virtual ~CommandAllocator() = default;

        void reset();

    protected:
        CommandAllocator(const CommandAllocator&) = default;
        CommandAllocator& operator=(const CommandAllocator&) = default;
        CommandAllocator(CommandAllocator&&) = default;
        CommandAllocator& operator=(CommandAllocator&&) = default;

    private:
        virtual void reset_api() = 0;
    };

    struct CommandAllocatorDesc {
    };

    struct CommandListDesc {
        const CommandAllocator* command_allocator;
    };
} // namespace orion
