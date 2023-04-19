#pragma once

#include "swapchain.h"

#include <variant> // std::variant

namespace orion
{
    using RenderTarget = std::variant<Swapchain>;
}
