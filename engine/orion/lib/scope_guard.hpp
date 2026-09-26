#pragma once

#include <exception>
#include <utility>

namespace orion
{
    enum class ScopeGuardPolicy {
        Exit = 0,
        Success,
        Fail,
    };

    template<ScopeGuardPolicy Policy, typename F>
    class ScopeGuard
    {
    public:
        explicit ScopeGuard(F&& func) noexcept
            : m_func(std::forward<F>(func))
        {
        }
        ScopeGuard(const ScopeGuard&)            = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;
        ScopeGuard(ScopeGuard&&)                 = delete;
        ScopeGuard& operator=(ScopeGuard&&)      = delete;
        ~ScopeGuard()
        {
            if constexpr (Policy == ScopeGuardPolicy::Exit) {
                m_func();
            } else if constexpr (Policy == ScopeGuardPolicy::Success) {
                if (!std::uncaught_exceptions()) {
                    m_func();
                }
            } else if constexpr (Policy == ScopeGuardPolicy::Fail) {
                if (std::uncaught_exceptions()) {
                    m_func();
                }
            }
        }

    private:
        F m_func;
    };

    template<typename F>
    using ScopeExit = ScopeGuard<ScopeGuardPolicy::Exit, F>;

    template<typename F>
    using ScopeSuccess = ScopeGuard<ScopeGuardPolicy::Success, F>;

    template<typename F>
    using ScopeFail = ScopeGuard<ScopeGuardPolicy::Fail, F>;
} // namespace orion
