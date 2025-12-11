#pragma once

#include "orion/assert.hpp"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

namespace orion
{
    template<typename T>
    class HandlePool
    {
    public:
        static constexpr auto grow_step = 64;

        struct Handle {
            std::uint32_t index;
            std::uint32_t generation;

            Handle(std::uint32_t idx, std::uint32_t gen)
                : index(idx)
                , generation(gen)
            {
            }

            Handle(std::uint64_t value) noexcept
                : index(value & 0xFFFFFFFF)
                , generation(value >> 32)
            {
            }

            constexpr std::uint64_t as_uint64_t() const noexcept
            {
                return static_cast<std::uint64_t>(generation) << 32 | index;
            }
        };

        Handle insert(T value)
        {
            if (freelist_.empty()) {
                grow();
            }
            const auto idx = claim_free_index();
            data_[idx] = std::move(value);
            // We could check for an overflow here, but 4B insert/destroy ops at a single index?
            ORION_ASSERT(generations_[idx] < UINT32_MAX, "Generation count has reached UINT32_MAX");
            const auto gen = ++generations_[idx];
            return Handle{idx, gen};
        }

        void remove(Handle handle)
        {
            const auto [index, generation] = handle;
            if (index >= data_.size()) {
                throw std::out_of_range("index out of range");
            }
            if (generation != generations_[index]) {
                throw std::invalid_argument("invalid generation for index");
            }
            data_[index] = std::nullopt;
            freelist_.push_back(index);
        }

        T* get(Handle handle)
        {
            const auto [index, generation] = handle;
            if (index >= data_.size()) {
                throw std::out_of_range("index out of range");
            }
            if (generation != generations_[index]) {
                throw std::invalid_argument("invalid generation for index");
            }

            if (data_[index].has_value()) {
                return &(*data_[index]);
            } else [[unlikely]] {
                return nullptr;
            }
        }

        const T* get(Handle handle) const
        {
            const auto [index, generation] = handle;
            if (index >= data_.size()) {
                throw std::out_of_range("index out of range");
            }
            if (generation != generations_[index]) {
                throw std::invalid_argument("invalid generation for index");
            }

            if (data_[index].has_value()) {
                return &(*data_[index]);
            } else [[unlikely]] {
                return nullptr;
            }
        }

    private:
        void grow()
        {
            const auto old_size = data_.size();
            const auto new_size = old_size + grow_step;
            if (new_size > UINT32_MAX) {
                throw std::runtime_error("Can't grow arena above UINT32_MAX elements");
            }

            data_.resize(new_size);
            generations_.resize(new_size);

            for (int i = static_cast<int>(new_size) - 1; i >= static_cast<int>(old_size); --i) {
                freelist_.push_back(static_cast<std::uint32_t>(i));
            }
        }

        std::uint32_t claim_free_index()
        {
            ORION_ASSERT(!freelist_.empty(), "Freelist of indices was empty.");
            const auto idx = freelist_.back();
            freelist_.pop_back();
            return idx;
        }

        std::vector<std::optional<T>> data_;
        std::vector<std::uint32_t> generations_;
        std::vector<std::uint32_t> freelist_;
    };
} // namespace orion
