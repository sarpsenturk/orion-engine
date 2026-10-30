#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace orion
{
    template<std::size_t Size, typename>
    class InplaceFunction;

    // Move-only callable with Size bytes of inline storage. Targets must fit and
    // require no more than max_align_t alignment. Empty invocation throws.
    template<std::size_t Size, typename Ret, typename... Args>
    class InplaceFunction<Size, Ret(Args...)>
    {
        static_assert(Size > 0);

        struct Operations {
            Ret (*invoke)(void*, Args&&...);
            void (*move)(void*, void*);
            void (*destroy)(void*) noexcept;
        };

        template<typename F>
        inline static constexpr Operations operations{
            [](void* storage, Args&&... args) -> Ret {
                auto& target = *std::launder(reinterpret_cast<F*>(storage));
                if constexpr (std::is_void_v<Ret>) {
                    std::invoke(target, std::forward<Args>(args)...);
                } else {
                    return std::invoke(target, std::forward<Args>(args)...);
                }
            },
            [](void* destination, void* source) {
                std::construct_at(reinterpret_cast<F*>(destination),
                    std::move(*std::launder(reinterpret_cast<F*>(source))));
            },
            [](void* storage) noexcept {
                std::destroy_at(std::launder(reinterpret_cast<F*>(storage)));
            }};

    public:
        InplaceFunction() noexcept = default;
        InplaceFunction(std::nullptr_t) noexcept {}

        template<typename F, typename Target = std::decay_t<F>>
            requires (!std::is_same_v<Target, InplaceFunction>
                && sizeof(Target) <= Size && alignof(Target) <= alignof(std::max_align_t)
                && std::is_constructible_v<Target, F>
                && std::is_move_constructible_v<Target>
                && std::is_nothrow_destructible_v<Target>
                && std::is_invocable_r_v<Ret, Target&, Args...>)
        InplaceFunction(F&& target)
        {
            if constexpr (std::is_pointer_v<Target> || std::is_member_pointer_v<Target>) {
                if (target == nullptr) {
                    return;
                }
            }
            std::construct_at(reinterpret_cast<Target*>(m_storage), std::forward<F>(target));
            m_operations = &operations<Target>;
        }

        InplaceFunction(const InplaceFunction&) = delete;
        InplaceFunction& operator=(const InplaceFunction&) = delete;

        InplaceFunction(InplaceFunction&& other)
        {
            move_from(other);
        }

        // If the target's move throws, this becomes empty and other remains valid.
        InplaceFunction& operator=(InplaceFunction&& other)
        {
            if (this != &other) {
                reset();
                move_from(other);
            }
            return *this;
        }

        InplaceFunction& operator=(std::nullptr_t) noexcept
        {
            reset();
            return *this;
        }

        ~InplaceFunction() { reset(); }

        explicit operator bool() const noexcept { return m_operations != nullptr; }

        Ret operator()(Args... args) const
        {
            if (!m_operations) {
                throw std::bad_function_call{};
            }
            return m_operations->invoke(m_storage, std::forward<Args>(args)...);
        }

        void reset() noexcept
        {
            if (m_operations) {
                m_operations->destroy(m_storage);
                m_operations = nullptr;
            }
        }

    private:
        void move_from(InplaceFunction& other)
        {
            if (other.m_operations) {
                other.m_operations->move(m_storage, other.m_storage);
                m_operations = other.m_operations;
                other.reset();
            }
        }

        alignas(std::max_align_t) mutable std::byte m_storage[Size];
        const Operations* m_operations = nullptr;
    };
} // namespace orion
