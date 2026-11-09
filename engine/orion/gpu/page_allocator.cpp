#include "page_allocator.hpp"

#include "orion/core/assert.hpp"
#include "orion/utils/scope_guard.hpp"

#include <algorithm>

namespace orion::gpu
{
    PageAllocator::PageAllocator(Device* device, const BufferDesc& page_desc)
        : m_device(device)
        , m_page_desc(page_desc)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(page_desc.size != 0z);
    }

    PageAllocator::~PageAllocator()
    {
        for (const auto& page : m_pages) { destroy_buffer(m_device, page.buffer); }
    }

    Buffer* PageAllocator::acquire(std::size_t min_size)
    {
        const auto size = std::max(m_page_desc.size, min_size);
        Page* best = nullptr;
        for (auto& page : m_pages) {
            if (!page.acquired && page.size >= size && (!best || page.size < best->size)) {
                best = &page;
            }
        }
        if (best) {
            best->acquired = true;
            return best->buffer;
        }

        auto* buffer = create_buffer(m_device, {.size = size, .memory = m_page_desc.memory});
        const auto guard = ScopeFail{[&] { destroy_buffer(m_device, buffer); }};
        m_pages.push_back({buffer, size, true});
        return buffer;
    }

    void PageAllocator::release(Buffer* buffer)
    {
        const auto page = std::find_if(m_pages.begin(), m_pages.end(),
                                       [buffer](const Page& entry) { return entry.buffer == buffer; });
        ORION_ASSERT(page != m_pages.end());
        if (page == m_pages.end()) { return; }
        ORION_ASSERT(page->acquired);
        page->acquired = false;
    }
} // namespace orion::gpu
