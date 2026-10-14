#include "deletion_queue.hpp"

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
        m_queue.push_back(std::make_pair(
            image,
            +[](gpu::Device* device, void* ptr) {
                gpu::destroy_image(device, static_cast<gpu::Image*>(ptr));
            }));
    }
    void DeletionQueue::push(gpu::ImageView* image_view)
    {
        m_queue.push_back(std::make_pair(
            image_view,
            +[](gpu::Device* device, void* ptr) {
                gpu::destroy_image_view(device, static_cast<gpu::ImageView*>(ptr));
            }));
    }

    void DeletionQueue::push(gpu::Buffer* buffer)
    {
        m_queue.push_back(std::make_pair(
            buffer,
            +[](gpu::Device* device, void* ptr) {
                gpu::destroy_buffer(device, static_cast<gpu::Buffer*>(ptr));
            }));
    }

    void DeletionQueue::flush()
    {
        for (const auto& [object, delete_fn] : m_queue) {
            delete_fn(m_device, object);
        }
        m_queue.clear();
    }
} // namespace orion
