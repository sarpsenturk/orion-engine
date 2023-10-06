#pragma once

#include "shader_compiler.h"

namespace orion
{
    class RenderDevice;

    class ShaderManager
    {
    public:
        explicit ShaderManager(RenderDevice* device);

    private:
        RenderDevice* device_;
    };
} // namespace orion
