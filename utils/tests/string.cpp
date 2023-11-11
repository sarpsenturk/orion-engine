#include "orion-utils/string.h"

#include <gtest/gtest.h>

namespace
{
    TEST(String, StringToWstring)
    {
        EXPECT_EQ(L"", orion::string_to_wstring(""));
        EXPECT_EQ(L"string", orion::string_to_wstring("string"));
        EXPECT_EQ(L"long test string for no SSO", orion::string_to_wstring("long test string for no SSO"));
    }

    TEST(String, WstringToString)
    {
        EXPECT_EQ("", orion::wstring_to_string(L""));
        EXPECT_EQ("string", orion::wstring_to_string(L"string"));
        EXPECT_EQ("long test string for no SSO", orion::wstring_to_string(L"long test string for no SSO"));
    }
} // namespace
