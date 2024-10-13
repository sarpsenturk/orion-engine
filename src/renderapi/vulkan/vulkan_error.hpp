#pragma once

#include <Volk/volk.h>

#include <source_location>

namespace orion
{
    const char* vk_result_string(VkResult result);

    void vk_assert(VkResult result, const char* message = "unexpected VkResult", const std::source_location& location = std::source_location::current());
} // namespace orion
