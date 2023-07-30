#pragma once

#include <algorithm>  // std::find_if
#include <functional> // std::function
#include <vector>     // std::vector

namespace orion
{
    template<typename T>
    class EventDispatcher;

    using handler_index = std::int64_t;

    template<typename Return, typename Event>
    class EventDispatcher<Return(Event)>
    {
    public:
        using handler_function = std::function<Return(Event)>;

        void invoke(const Event& event) const
        {
            for (const auto& [_, handler] : handlers_) {
                handler(event);
            }
        }

        template<typename Callable>
            requires std::is_same_v<std::invoke_result_t<Callable, Event>, Return>
        handler_index subscribe(Callable&& callable)
        {
            auto index = index_++;
            handlers_.emplace_back(index, std::forward<Callable>(callable));
            return index;
        };

        void unsubscribe(handler_index index)
        {
            auto iter = std::ranges::find_if(handlers_, [index](const auto& handler) { return handler.first == index; });
            if (iter != handlers_.end()) {
                handlers_.erase(iter);
            }
        }

        template<typename Callable>
        handler_index operator+=(Callable&& callable)
        {
            return subscribe(std::forward<Callable>(callable));
        }

        void operator-=(handler_index index)
        {
            return unsubscribe(index);
        }

        [[nodiscard]] bool has_handlers() const noexcept { return !handlers_.empty(); }
        [[nodiscard]] auto handler_count() const noexcept { return handlers_.size(); }

    private:
        handler_index index_ = 0;
        std::vector<std::pair<handler_index, handler_function>> handlers_;
    };
} // namespace orion
