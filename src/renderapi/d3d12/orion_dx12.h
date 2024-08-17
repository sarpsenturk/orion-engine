#pragma once

#include <wrl/client.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>

#include <string>

namespace orion
{
    using Microsoft::WRL::ComPtr;

    void hr_assert(HRESULT hr, const char* msg = "unexpected HRESULT");

    std::string wstring_to_string(const wchar_t* wstr);
} // namespace orion
