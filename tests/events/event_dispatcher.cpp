#include "orion/events/event_dispatcher.h"

#include <gtest/gtest.h>

namespace
{
    class SimpleDispatcher : public orion::EventDispatcher<int, float, double>
    {
    };

    class IntHandler final : public orion::EventHandler<int>
    {
    public:
        void process(const int& event) override { event_value = event; }
        int value() const noexcept { return event_value; }

    private:
        int event_value = -1;
    };

    TEST(EventDispatcher, Attach)
    {
        SimpleDispatcher dispatcher;
        IntHandler handler;
        dispatcher.attach<int>(&handler);
        EXPECT_FALSE(dispatcher.empty<int>());
        auto& int_handlers = dispatcher.event_handlers<int>();
        EXPECT_EQ(int_handlers[0], &handler);
    }

    TEST(EventDispatcher, Detach)
    {
        SimpleDispatcher dispatcher;
        IntHandler handler;
        dispatcher.attach<int>(&handler);
        EXPECT_FALSE(dispatcher.empty<int>());
        dispatcher.detach<int>(&handler);
        EXPECT_TRUE(dispatcher.empty<int>());
    }

    TEST(EventDispatcher, Notify)
    {
        SimpleDispatcher dispatcher;
        IntHandler handler;
        dispatcher.attach<int>(&handler);
        constexpr int expected = 42;
        dispatcher.notify(expected);
        EXPECT_EQ(expected, handler.value());
    }
} // namespace
