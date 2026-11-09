#pragma once

#include "gpu.hpp"

#include <vector>

namespace orion::gpu
{
    // Owns buffer pages. Device must outlive the allocator. All CPU and GPU
    // synchronization is external, including before release and destruction.
    class PageAllocator
    {
    public:
        // The default page size must be nonzero. All pages use page_desc.memory.
        explicit PageAllocator(Device* device, const BufferDesc& page_desc);
        PageAllocator(const PageAllocator&) = delete;
        PageAllocator& operator=(const PageAllocator&) = delete;
        PageAllocator(PageAllocator&&) = delete;
        PageAllocator& operator=(PageAllocator&&) = delete;
        // Destroys all pages, including those still acquired by callers.
        ~PageAllocator();

        // Reuses the smallest free page that fits, or creates a page sized
        // max(default page size, min_size). Zero requests the default page size.
        [[nodiscard]] Buffer* acquire(std::size_t min_size = 0z);
        // Buffer must be a currently acquired page from this allocator.
        // Retains the buffer for reuse; its contents are not cleared.
        void release(Buffer* buffer);

    private:
        struct Page {
            Buffer* buffer;
            std::size_t size;
            bool acquired;
        };

        Device* m_device;
        BufferDesc m_page_desc;
        std::vector<Page> m_pages;
    };
} // namespace orion::gpu
