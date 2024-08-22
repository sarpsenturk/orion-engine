#pragma once

#include <Volk/volk.h>

namespace orion
{
    const char* vk_result_string(VkResult result);

    void vk_assert(VkResult result, const char* message = "unexpected VkResult");
} // namespace orion
