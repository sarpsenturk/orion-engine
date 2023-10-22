#pragma once

#include "allocator.h"

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace orion
{
    class LinearAllocator
    {
    public:
        LinearAllocator()
            : buffer_(0)
        {
        }

        explicit LinearAllocator(std::size_t max_size)
            : buffer_(max_size)
        {
        }

        LinearAllocator(const LinearAllocator& other)
            : buffer_(other.buffer_)
            , current_(buffer_.data())
            , end_(buffer_.data() + buffer_.capacity())
        {
        }

        LinearAllocator(LinearAllocator&& other) noexcept
            : buffer_(std::move(other.buffer_))
            , current_(buffer_.data())
            , end_(buffer_.data() + buffer_.capacity())
        {
        }

        LinearAllocator& operator=(const LinearAllocator& other)
        {
            if (&other != this) {
                buffer_ = other.buffer_;
                current_ = buffer_.data();
                end_ = buffer_.data() + buffer_.capacity();
            }
            return *this;
        }

        LinearAllocator& operator=(LinearAllocator&& other) noexcept
        {
            if (&other != this) {
                buffer_ = std::move(other.buffer_);
                current_ = buffer_.data();
                end_ = buffer_.data() + buffer_.capacity();
            }
            return *this;
        }

        ~LinearAllocator() = default;

        [[nodiscard]] auto max_size() const noexcept { return buffer_.capacity(); }
        [[nodiscard]] auto space() const noexcept { return end_ - current_; }
        [[nodiscard]] auto is_full() const noexcept { return current_ == end_; }
        [[nodiscard]] auto is_empty() const noexcept { return current_ == buffer_.data(); }

        Allocation allocate(std::size_t size, std::size_t alignment) noexcept
        {
            std::size_t remaining_space = space();
            void* ptr = current_;
            if (std::align(alignment, size, ptr, remaining_space)) {
                current_ = static_cast<char*>(ptr) + size;
                return {.ptr = ptr, .size = size};
            }
            return {.ptr = nullptr, .size = 0};
        }

        void deallocate(Allocation allocation)
        {
            (void)allocation;
            throw std::logic_error("Cannot deallocate individual allocations in linear allocator");
        }

        void reset()
        {
            current_ = buffer_.data();
        }

    private:
        std::vector<char> buffer_;
        char* current_ = buffer_.data();
        char* end_ = buffer_.data() + buffer_.capacity();
    };
} // namespace orion
