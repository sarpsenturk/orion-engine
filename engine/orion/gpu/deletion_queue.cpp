#include "deletion_queue.hpp"

#include "orion/core/assert.hpp"

#include <utility>

namespace orion::gpu
{
    void DeletionQueue::push(Callback callback)
    {
        ORION_ASSERT(callback && "DeletionQueue requires a nonempty callback");
        m_callbacks.push_back(std::move(callback));
    }

    void DeletionQueue::flush() noexcept
    {
        for (auto& callback : m_callbacks) {
            callback();
        }
        m_callbacks.clear();
    }
} // namespace orion::gpu
