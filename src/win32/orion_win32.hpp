#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <string>

namespace orion::win32
{
    std::string format_last_error(DWORD last_error = GetLastError());
}
