#pragma once

#include "orion/lib/inline_function.hpp"
#include "orion/renderer/gpu/gpu.hpp"

#include <vector>

namespace orion
{
    class DeletionQueue
    {
    public:
        using DeleteFn = InlineFunction<64, void(gpu::Device*)>;

        explicit DeletionQueue(gpu::Device* device);
        DeletionQueue(const DeletionQueue&)            = delete;
        DeletionQueue& operator=(const DeletionQueue&) = delete;
        DeletionQueue(DeletionQueue&&)                 = delete;
        DeletionQueue& operator=(DeletionQueue&&)      = delete;
        ~DeletionQueue();

        void push(gpu::Image* image);
        void push(gpu::ImageView* image_view);
        void push(gpu::Buffer* buffer);
        void push(DeleteFn callback);
        void flush();

    private:
        gpu::Device* m_device;
        std::vector<DeleteFn> m_queue;
    };
} // namespace orion
