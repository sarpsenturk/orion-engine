#include "deletion_queue.hpp"

#include <utility>

namespace orion
{
    DeletionQueue::DeletionQueue(gpu::Device* device)
        : m_device(device)
    {
    }

    DeletionQueue::~DeletionQueue()
    {
        flush();
    }

    void DeletionQueue::push(gpu::Image* image)
    {
        push([image](gpu::Device* device) { gpu::destroy_image(device, image); });
    }
    void DeletionQueue::push(gpu::ImageView* image_view)
    {
        push([image_view](gpu::Device* device) { gpu::destroy_image_view(device, image_view); });
    }

    void DeletionQueue::push(gpu::Buffer* buffer)
    {
        push([buffer](gpu::Device* device) { gpu::destroy_buffer(device, buffer); });
    }

    void DeletionQueue::push(DeleteFn callback)
    {
        m_queue.push_back(std::move(callback));
    }

    void DeletionQueue::flush()
    {
        for (const auto& delete_fn : m_queue) {
            delete_fn(m_device);
        }
        m_queue.clear();
    }
} // namespace orion
