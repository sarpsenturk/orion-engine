#include "orion/orion.h"

#include <gtest/gtest.h>

namespace {
    TEST(Basic, Test)
    {
        EXPECT_EQ(0, orion::return_ok());
    }
}
