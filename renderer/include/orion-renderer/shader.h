#pragma once

#include "shader_compiler.h"

#include "orion-renderapi/render_device.h"
#include "orion-renderapi/types.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <spdlog/logger.h>

namespace orion
{
    inline constexpr auto vs_entry_point = "vs_main";
    inline constexpr auto fs_entry_point = "fs_main";

    class Shader
    {
    public:
        Shader(UniqueShaderModule vertex_shader, UniqueShaderModule fragment_shader);

        [[nodiscard]] auto vertex() const noexcept { return vertex_shader_.get(); }
        [[nodiscard]] auto fragment() const noexcept { return fragment_shader_.get(); }

    private:
        std::vector<ShaderStageDesc> make_pipeline_desc() const;

        UniqueShaderModule vertex_shader_;
        UniqueShaderModule fragment_shader_;
        std::vector<ShaderStageDesc> pipeline_shader_desc_;
    };

    class ShaderManager
    {
    public:
        explicit ShaderManager(RenderDevice* device);

        const Shader* add_from_source(const std::string& name, const std::string& source, ShaderStageFlags shader_stages);
        const Shader* add_from_file(const std::string& name, const std::string& filename, ShaderStageFlags shader_stages);
        void remove(const std::string& name);

    private:
        const Shader* add(const std::string& name, const std::string& source_str, ShaderStageFlags shader_stages, shader_compiler_compile_fn compile_fn);

        RenderDevice* device_;
        ShaderObjectType object_type_;
        std::unordered_map<std::string, Shader> shaders_;
        ShaderCompiler shader_compiler_;

        static spdlog::logger* logger();
    };
} // namespace orion
