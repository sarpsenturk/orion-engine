#pragma once

#include <cstdint>
#include <unordered_map>
#include <functional>

namespace orion
{
    template<typename E>
    class Event
    {
    public:
        using EventType = E;
        using HandlerId = std::int32_t;
        using HandlerFn = std::function<bool(const EventType&)>;

        void invoke(const EventType& event)
        {
            for (const auto& [_, handler] : handlers_) {
                if (handler(event)) {
                    break;
                }
            }
        }

        template<typename F>
        HandlerId subscribe(F&& fn)
        {
            const auto id = next_id++;
            handlers_.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(fn));
            return id;
        }

        void unsubscribe(HandlerId id)
        {
            handlers_.erase(id);
        }

    private:
        HandlerId next_id = 1;
        std::unordered_map<HandlerId, HandlerFn> handlers_;
    };
} // namespace orion
