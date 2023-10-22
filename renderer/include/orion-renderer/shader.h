#pragma once

#include "shader_compiler.h"

#include "orion-renderapi/handles.h"

#include <string_view>

namespace orion
{
    class RenderDevice;

    class ShaderManager
    {
    public:
        static constexpr auto vs_entry_point = "vs_main";
        static constexpr auto fs_entry_point = "fs_main";

        explicit ShaderManager(RenderDevice* device);

        ShaderModuleHandle create_shader_module(std::string_view source, const char* entry_point, ShaderStageFlags stage) const;
        ShaderModuleHandle create_vs(std::string_view source) const;
        ShaderModuleHandle create_fs(std::string_view source) const;

    private:
        RenderDevice* device_;
        ShaderObjectType object_type_;
        ShaderCompiler shader_compiler_;
    };
} // namespace orion
