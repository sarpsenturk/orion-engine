#include "orion-scene/components.h"

#include "orion-math/matrix/transformation.h"

#include <gtest/gtest.h>

namespace
{
    TEST(TransformComponent, Position)
    {
        // Empty transform
        {
            const auto transform = orion::TransformComponent{};
            EXPECT_EQ(transform.position(), orion::Vector3_f{});
        }

        // Translate
        {
            const auto transform = orion::TransformComponent{.transform = orion::translation(10.f, 0.f, 5.f)};
            const auto expected = orion::Vector3_f{10.f, 0.f, 5.f};
            EXPECT_EQ(transform.position(), expected);
        }
    }

    TEST(TransformComponent, Translate)
    {
        auto transform = orion::TransformComponent{};
        transform.translate({10.f, 0.5f, 5.f});
        const auto expected = orion::Vector3_f{10.f, 0.5f, 5.f};
        EXPECT_EQ(transform.position(), expected);
    }
} // namespace
