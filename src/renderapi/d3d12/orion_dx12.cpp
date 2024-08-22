#include "orion_dx12.h"

#include <fmt/format.h>

#include <stdexcept>

namespace orion
{
    void hr_assert(HRESULT hr, const char* msg, const std::source_location& location)
    {
        if (FAILED(hr)) [[unlikely]] {
            throw std::runtime_error{fmt::format("{}:{} {}: {:#x}", location.file_name(), location.line(), msg, static_cast<std::make_unsigned_t<HRESULT>>(hr))};
        }
    }

    std::string wstring_to_string(const wchar_t* wstr)
    {
        const auto size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
        std::string utf8(size, '\0');
        if (WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &utf8[0], size, nullptr, nullptr) == 0) [[unlikely]] {
            throw std::runtime_error{"failed to convert wstring to string"};
        }
        return utf8;
    }
} // namespace orion
