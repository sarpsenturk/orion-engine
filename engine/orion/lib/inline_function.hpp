#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace orion
{
    template<std::size_t, typename>
    class InlineFunction;

    template<std::size_t Capacity, typename R, typename... Args>
    class InlineFunction<Capacity, R(Args...)>
    {
    public:
        static_assert(Capacity > 0, "Capacity must be greater than 0");

        using result_type = R;

        static constexpr auto capacity  = Capacity;
        static constexpr auto alignment = alignof(std::max_align_t);

        constexpr InlineFunction() noexcept = default;

        template<typename F>
            requires(
                std::is_invocable_r_v<R, std::decay_t<F>&, Args...> &&
                sizeof(std::decay_t<F>) <= Capacity &&
                alignof(std::decay_t<F>) <= alignment &&
                std::is_nothrow_constructible_v<std::decay_t<F>, F &&> &&
                std::is_nothrow_move_constructible_v<std::decay_t<F>> &&
                std::is_nothrow_destructible_v<std::decay_t<F>>)
        InlineFunction(F&& callable)
        {
            using T = std::decay_t<F>;

            // Create the callable in place in storage
            std::construct_at(reinterpret_cast<T*>(m_storage), std::forward<F>(callable));

            // Setup operations
            m_invoke = [](void* storage, Args&&... args) -> R {
                return std::invoke(*static_cast<T*>(storage), std::forward<Args>(args)...);
            };
            m_move = [](void* dest, void* src) noexcept {
                auto* source = static_cast<T*>(src);

                std::construct_at(
                    static_cast<T*>(dest),
                    std::move(*source));

                std::destroy_at(source);
            };
            m_destroy = [](void* storage) noexcept {
                std::destroy_at(static_cast<T*>(storage));
            };
        }

        InlineFunction(const InlineFunction&)            = delete;
        InlineFunction& operator=(const InlineFunction&) = delete;

        InlineFunction(InlineFunction&& other) noexcept
            : m_invoke(std::exchange(other.m_invoke, nullptr))
            , m_move(std::exchange(other.m_move, nullptr))
            , m_destroy(std::exchange(other.m_destroy, nullptr))
        {
            if (m_invoke) {
                m_move(&m_storage[0], &other.m_storage[0]);
            }
        }

        InlineFunction& operator=(InlineFunction&& other) noexcept
        {
            if (this != &other) {
                reset();
                m_invoke  = std::exchange(other.m_invoke, nullptr);
                m_move    = std::exchange(other.m_move, nullptr);
                m_destroy = std::exchange(other.m_destroy, nullptr);
                if (m_invoke) {
                    m_move(&m_storage[0], &other.m_storage[0]);
                }
            }
            return *this;
        }

        ~InlineFunction() noexcept { reset(); }

        R operator()(Args... args) const
        {
            if (!m_invoke) {
                throw std::bad_function_call();
            }
            return m_invoke(const_cast<void*>(static_cast<const void*>(&m_storage[0])), std::forward<Args>(args)...);
        }

        [[nodiscard]] bool empty() const noexcept { return m_invoke == nullptr; }
        [[nodiscard]] explicit operator bool() const noexcept { return !empty(); }

        void reset() noexcept
        {
            if (m_destroy) {
                m_destroy(&m_storage[0]);
                m_invoke  = nullptr;
                m_move    = nullptr;
                m_destroy = nullptr;
            }
        }

    private:
        using invoke_function_t  = R (*)(void*, Args&&...);
        using move_function_t    = void (*)(void*, void*) noexcept;
        using destroy_function_t = void (*)(void*) noexcept;

        alignas(alignment) std::byte m_storage[Capacity];

        invoke_function_t m_invoke   = nullptr;
        move_function_t m_move       = nullptr;
        destroy_function_t m_destroy = nullptr;
    };
} // namespace orion
