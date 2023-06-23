#include "orion-math/angles.h"

#include <gtest/gtest.h>

namespace
{
    constexpr auto acceptable_error = 1e-5;

    TEST(Angles, UnderlyingValue)
    {
        constexpr auto expected = 42;
        const orion::Angle angle{expected};
        EXPECT_EQ(angle.value(), expected);
    }

    TEST(Angles, DegreeToRadian)
    {
        const orion::Degrees degrees{1};
        const orion::Radians radians{degrees};
        EXPECT_NEAR(radians.value(), 0.01745329251, acceptable_error);
    }

    TEST(Angles, RadianToDegree)
    {
        const orion::Radians radians{1};
        const orion::Degrees degrees{radians};
        EXPECT_NEAR(degrees.value(), 57.2957795131, acceptable_error);
    }

    TEST(Angles, Pi)
    {
        const orion::Radians radians{orion::pi};
        const orion::Degrees degrees{radians};
        EXPECT_NEAR(degrees.value(), 180.0, acceptable_error);
    }

    TEST(Angles, AddSame)
    {
        const orion::Radians radians{1};
        EXPECT_EQ((radians + radians), orion::Radians{2});
    }

    TEST(Angles, AddDifferent)
    {
        const orion::Radians radians{1};
        const orion::Degrees degrees{180};
        const auto result = radians + degrees;
        EXPECT_EQ(result, orion::Radians{4.141594});
        EXPECT_EQ(result, orion::Degrees{237.2957});
    }

    TEST(Angles, SubtractSame)
    {
        const orion::Radians radians{1};
        EXPECT_EQ((radians - radians), orion::Radians{0});
    }

    TEST(Angles, SubtractDifferent)
    {
        const orion::Radians radians{5};
        const orion::Degrees degrees{180};
        const auto result = radians - degrees;
        EXPECT_EQ(result, orion::Radians{1.85841});
        EXPECT_EQ(result, orion::Degrees{106.4790496});
    }

    TEST(Angles, Multiply)
    {
        const orion::Degrees degrees{15};
        const orion::Degrees expected{30};
        EXPECT_EQ((degrees * 2), expected);
    }

    TEST(Angles, Divide)
    {
        const orion::Degrees degrees{30};
        const orion::Degrees expected{15};
        EXPECT_EQ((degrees / 2), expected);
    }

    TEST(Angles, Negate)
    {
        constexpr auto value = 42;
        const orion::Degrees degrees{value};
        const orion::Radians radians{value};
        EXPECT_EQ((-degrees).value(), -value);
        EXPECT_EQ((-radians).value(), -value);
    }
} // namespace
