#pragma once

#include "shader_compiler.h"

#include "orion-renderapi/render_device.h"
#include "orion-renderapi/types.h"

#include <cstdint>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
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
        [[nodiscard]] auto pipeline_shader_desc() const noexcept { return std::span{pipeline_shader_desc_}; }

    private:
        [[nodiscard]] std::vector<ShaderStageDesc> make_pipeline_desc() const;

        UniqueShaderModule vertex_shader_;
        UniqueShaderModule fragment_shader_;
        std::vector<ShaderStageDesc> pipeline_shader_desc_;
    };

    using shader_handle_key = std::uint16_t;
    ORION_DEFINE_HANDLE(ShaderHandle, shader_handle_key);

    class ShaderManager
    {
    public:
        explicit ShaderManager(RenderDevice* device);

        std::pair<ShaderHandle, const Shader*> add_from_source(const std::string& source, ShaderStageFlags shader_stages);
        std::pair<ShaderHandle, const Shader*> add_from_file(const std::string& filename, ShaderStageFlags shader_stages);
        void remove(ShaderHandle shader_handle) noexcept;

    private:
        ShaderHandle next_handle() noexcept { return ShaderHandle{shader_index_++}; }
        std::pair<ShaderHandle, const Shader*> add(const std::string& source_str, ShaderStageFlags shader_stages, shader_compiler_compile_fn compile_fn);

        RenderDevice* device_;
        ShaderObjectType object_type_;
        std::unordered_map<ShaderHandle, Shader> shaders_;
        ShaderCompiler shader_compiler_;

        shader_handle_key shader_index_ = 0;

        static spdlog::logger* logger();
    };
} // namespace orion
