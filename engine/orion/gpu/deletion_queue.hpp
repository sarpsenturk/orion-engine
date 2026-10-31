#pragma once

#include "orion/utils/inplace_function.hpp"

#include <vector>

namespace orion::gpu
{
    // Single-threaded deferred cleanup. The caller must ensure GPU work has
    // completed before flushing. Callbacks must not throw or mutate this queue.
    class DeletionQueue
    {
    public:
        using Callback = InplaceFunction<64, void()>;

        // Takes ownership of a nonempty callback.
        void push(Callback callback);
        // Executes callbacks in insertion order and retains storage for reuse.
        void flush() noexcept;
        [[nodiscard]] bool empty() const noexcept { return m_callbacks.empty(); }

    private:
        std::vector<Callback> m_callbacks;
    };
} // namespace orion::gpu
