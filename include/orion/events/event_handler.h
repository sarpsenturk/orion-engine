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
        // TODO: Add passing trivial types by value back
        virtual void process(const EventType& event) = 0;
    };
} // namespace orion

#endif // ORION_ENGINE_EVENT_HANDLER_H
