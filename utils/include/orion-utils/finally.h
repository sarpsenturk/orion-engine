#pragma once

#include <utility>

namespace orion
{
    template<typename F>
    class FinalAction
    {
    public:
        explicit FinalAction(F&& action) noexcept
            : action_(std::forward<F>(action))
        {
        }
        FinalAction(const FinalAction&) = delete;
        FinalAction(FinalAction&&) = delete;
        void operator=(const FinalAction&) = delete;
        void operator=(FinalAction&&) = delete;
        ~FinalAction() { action_(); }

    private:
        F action_;
    };

    template<typename F>
    auto finally(F&& action)
    {
        return FinalAction{std::forward<F>(action)};
    }
} // namespace orion
