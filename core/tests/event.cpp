#include "orion-core/event.h"

#include <gtest/gtest.h>

namespace
{
    TEST(Event, DefaultCtor)
    {
        const orion::EventDispatcher<void(int)> dispatcher;
        EXPECT_FALSE(dispatcher.has_handlers());
        EXPECT_EQ(dispatcher.handler_count(), 0);
    }

    TEST(Event, Subscribe)
    {
        orion::EventDispatcher<void(int)> dispatcher;
        auto callback = [](int) {};
        dispatcher.subscribe(callback);
        EXPECT_TRUE(dispatcher.has_handlers());
        EXPECT_EQ(dispatcher.handler_count(), 1);
    }

    TEST(Event, SubscribePlusEquals)
    {
        orion::EventDispatcher<void(int)> dispatcher;
        auto callback = [](int) {};
        dispatcher += callback;
        EXPECT_TRUE(dispatcher.has_handlers());
        EXPECT_EQ(dispatcher.handler_count(), 1);
    }

    TEST(Event, Unsubscribe)
    {
        orion::EventDispatcher<void(int)> dispatcher;
        auto callback = [](int) {};
        auto index = dispatcher += callback;
        dispatcher.unsubscribe(index);
        EXPECT_EQ(dispatcher.handler_count(), 0);
    }

    TEST(Event, UnsubscribeMinusEquals)
    {
        orion::EventDispatcher<void(int)> dispatcher;
        auto callback = [](int) {};
        auto index = dispatcher += callback;
        dispatcher -= index;
        EXPECT_EQ(dispatcher.handler_count(), 0);
    }

    TEST(Event, InvokeEmpty)
    {
        const orion::EventDispatcher<void(int)> dispatcher;
        constexpr int add = 42;
        int value = 0;
        auto callback = [&value](int i) { value += i; };
        dispatcher.invoke(add);
        EXPECT_EQ(value, 0);
    }

    TEST(Event, InvokeSingle)
    {
        orion::EventDispatcher<void(int)> dispatcher;
        constexpr int add = 42;
        int value = 0;
        auto callback = [&value](int i) { value += i; };
        dispatcher.subscribe(callback);
        dispatcher.invoke(add);
        EXPECT_EQ(value, add);
    }

    TEST(Event, InvokeMultiple)
    {
        orion::EventDispatcher<void(int)> dispatcher;
        constexpr int add = 42;
        constexpr int times = 3;
        int value = 0;
        auto callback = [&value](int i) { value += i; };
        for (int i = 0; i < times; ++i) {
            dispatcher.subscribe(callback);
        }
        dispatcher.invoke(add);
        EXPECT_EQ(value, add * times);
    }
} // namespace
