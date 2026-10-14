#pragma once

#include "orion/renderer/gpu/gpu.hpp"

#include <utility>
#include <vector>

namespace orion
{
    class DeletionQueue
    {
    public:
        using DeleteFn = void (*)(gpu::Device*, void*);

        explicit DeletionQueue(gpu::Device* device);
        DeletionQueue(const DeletionQueue&)            = delete;
        DeletionQueue& operator=(const DeletionQueue&) = delete;
        DeletionQueue(DeletionQueue&&)                 = delete;
        DeletionQueue& operator=(DeletionQueue&&)      = delete;
        ~DeletionQueue();

        void push(gpu::Image* image);
        void push(gpu::ImageView* image_view);
        void push(gpu::Buffer* buffer);
        void flush();

    private:
        gpu::Device* m_device;
        std::vector<std::pair<void*, DeleteFn>> m_queue;
    };
} // namespace orion
