#include "orion-math/angles.h"

#include <gtest/gtest.h>

namespace
{
    constexpr auto acceptable_error = 1e-5;

    TEST(Angles, UnderlyingValue)
    {
        constexpr auto expected = 42;
        const orion::math::Angle angle{expected};
        EXPECT_EQ(angle.value(), expected);
    }

    TEST(Angles, DegreeToRadian)
    {
        const orion::math::Degrees degrees{1};
        const orion::math::Radians radians{degrees};
        EXPECT_NEAR(radians.value(), 0.01745329251, acceptable_error);
    }

    TEST(Angles, RadianToDegree)
    {
        const orion::math::Radians radians{1};
        const orion::math::Degrees degrees{radians};
        EXPECT_NEAR(degrees.value(), 57.2957795131, acceptable_error);
    }

    TEST(Angles, Pi)
    {
        const orion::math::Radians radians{orion::math::pi};
        const orion::math::Degrees degrees{radians};
        EXPECT_NEAR(degrees.value(), 180.0, acceptable_error);
    }

    TEST(Angles, AddSame)
    {
        const orion::math::Radians radians{1};
        EXPECT_EQ((radians + radians), orion::math::Radians{2});
    }

    TEST(Angles, AddDifferent)
    {
        const orion::math::Radians radians{1};
        const orion::math::Degrees degrees{180};
        const auto result = radians + degrees;
        EXPECT_EQ(result, orion::math::Radians{4.141594});
        EXPECT_EQ(result, orion::math::Degrees{237.2957});
    }

    TEST(Angles, SubtractSame)
    {
        const orion::math::Radians radians{1};
        EXPECT_EQ((radians - radians), orion::math::Radians{0});
    }

    TEST(Angles, SubtractDifferent)
    {
        const orion::math::Radians radians{5};
        const orion::math::Degrees degrees{180};
        const auto result = radians - degrees;
        EXPECT_EQ(result, orion::math::Radians{1.85841});
        EXPECT_EQ(result, orion::math::Degrees{106.4790496});
    }

    TEST(Angles, Multiply)
    {
        const orion::math::Degrees degrees{15};
        const orion::math::Degrees expected{30};
        EXPECT_EQ((degrees * 2), expected);
    }

    TEST(Angles, Divide)
    {
        const orion::math::Degrees degrees{30};
        const orion::math::Degrees expected{15};
        EXPECT_EQ((degrees / 2), expected);
    }

    TEST(Angles, Negate)
    {
        constexpr auto value = 42;
        const orion::math::Degrees degrees{value};
        const orion::math::Radians radians{value};
        EXPECT_EQ((-degrees).value(), -value);
        EXPECT_EQ((-radians).value(), -value);
    }
} // namespace
