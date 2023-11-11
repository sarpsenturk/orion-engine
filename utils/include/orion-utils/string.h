#pragma once

#include <codecvt>
#include <string>

namespace orion
{
    inline std::wstring string_to_wstring(const std::string& string)
    {
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4996)
#endif
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(string);
#ifdef _MSC_VER
    #pragma warning(pop)
#endif
    }

    inline std::string wstring_to_string(const std::wstring& wstring)
    {
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4996)
#endif
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(wstring);
#ifdef _MSC_VER
    #pragma warning(pop)
#endif
    }
} // namespace orion
