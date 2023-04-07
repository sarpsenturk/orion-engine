#pragma once

#include "orion-core/handle.h"

/*
 * All handles to be used in the render backend should be defined here.
 * This is to ensure we don't get odr violations.
 */

namespace orion
{
    ORION_DEFINE_HANDLE(SwapchainHandle);
} // namespace orion
