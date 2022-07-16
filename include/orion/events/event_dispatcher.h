#ifndef ORION_ENGINE_EVENT_DISPATCHER_H
#define ORION_ENGINE_EVENT_DISPATCHER_H

#include "event_handler.h"
#include "orion/utility/index_of.h"

#include <array>
#include <vector>

namespace orion
{
    template<typename... EventTypes>
    class EventDispatcher
    {
    public:
        using EventHandlerVec_t = std::vector<IEventHandler*>;

    public:
        template<typename EventType>
        void attach(EventHandler<EventType>* handler)
        {
            event_handlers<EventType>().push_back(handler);
        }

        template<typename EventType>
        void detach(EventHandler<EventType>* handler)
        {
            auto& handlers = event_handlers<EventType>();
            if (auto it = std::ranges::find(handlers, handler);
                it != handlers.end()) {
                handlers.erase(it);
            }
        }

        template<typename EventType>
        void notify(const EventType& event) const
        {
            for (auto handler : event_handlers<EventType>()) {
                auto typed_handler =
                    static_cast<EventHandler<EventType>*>(handler);
                typed_handler->process(event);
            }
        }

        template<typename EventType>
        [[nodiscard]] constexpr EventHandlerVec_t& event_handlers() noexcept
        {
            constexpr auto index = detail::index_of_v<EventType, EventTypes...>;
            return handlers_[index];
        }

        template<typename EventType>
        [[nodiscard]] constexpr const EventHandlerVec_t&
        event_handlers() const noexcept
        {
            constexpr auto index = detail::index_of_v<EventType, EventTypes...>;
            return handlers_[index];
        }

        template<typename EventType>
        [[nodiscard]] bool empty() const noexcept
        {
            return event_handlers<EventType>().empty();
        }

    private:
        std::array<EventHandlerVec_t, sizeof...(EventTypes)> handlers_;
    };
} // namespace orion

#endif // ORION_ENGINE_EVENT_DISPATCHER_H
