#include "orion/lib/hash.hpp"

#include <gtest/gtest.h>

namespace orion
{
    namespace
    {
        TEST(Hash, FNV1a)
        {
            EXPECT_EQ(fnv1a_hash(""), 0xcbf29ce484222325ull);
            EXPECT_EQ(fnv1a_hash("hello"), 0xa430d84680aabd0bull);
            EXPECT_EQ(fnv1a_hash("foobar"), 0x85944171f73967e8ull);
        }
    } // namespace
} // namespace orion
