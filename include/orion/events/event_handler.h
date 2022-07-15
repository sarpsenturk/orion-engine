#ifndef ORION_ENGINE_EVENT_HANDLER_H
#define ORION_ENGINE_EVENT_HANDLER_H

#include <type_traits>

namespace orion
{
    class IEventHandler
    {
    };

    template<typename EventType>
    class EventHandler : public IEventHandler
    {
    public:
        virtual ~EventHandler() = default;
        using EventTypeNoRef = std::remove_reference_t<EventType>;
        using EventTypeLvalueRef = std::add_lvalue_reference_t<EventType>;
        static constexpr bool kIsTrivialEvent =
            std::is_trivial_v<EventTypeNoRef>;
        using EventParam_t = std::conditional_t<kIsTrivialEvent, EventTypeNoRef,
                                                EventTypeLvalueRef>;
        virtual void process(const EventParam_t event) = 0;
    };
} // namespace orion

#endif // ORION_ENGINE_EVENT_HANDLER_H
