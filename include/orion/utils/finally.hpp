#pragma once

#include <utility>

namespace orion
{
    template<typename F>
    class Finally
    {
    public:
        Finally(F&& func)
            : func_(std::forward<F>(func))
        {
        }
        Finally(const Finally&) = delete;
        Finally& operator=(const Finally&) = delete;
        Finally(Finally&&) = delete;
        Finally& operator=(Finally&&) = delete;
        ~Finally() { func_(); }

    private:
        F func_;
    };
} // namespace orion
