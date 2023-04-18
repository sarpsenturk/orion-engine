#include "orion-renderapi/shader.h"

namespace orion
{
    ShaderModule::ShaderModule(ShaderModuleHandleRef handle)
        : handle_(std::move(handle))
    {
    }
} // namespace orion
