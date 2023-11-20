#pragma once

#include "orion-assets/config.h"

#include "orion-core/filesystem.h"
#include "orion-core/handle.h"

#include "orion-renderapi/defs.h"

#include <spdlog/logger.h>

#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace orion
{
    class Shader
    {
    public:
        Shader(std::string name, ShaderModuleHandle shader_module);

        [[nodiscard]] auto& name() const { return name_; }
        [[nodiscard]] auto shader_module() const { return shader_module_; }

    private:
        std::string name_;
        ShaderModuleHandle shader_module_;
    };

    class ShaderEffect
    {
    public:
        static constexpr auto pipeline_size = 2ull;

        explicit ShaderEffect(std::array<const Shader*, pipeline_size> shaders);

        [[nodiscard]] auto* vertex_shader() const { return shaders_[0]; }
        [[nodiscard]] auto* pixel_shader() const { return shaders_[1]; }

        [[nodiscard]] std::span<const ShaderStageDesc> shader_stages() const { return std::span{shader_stages_}; }

    private:
        [[nodiscard]] std::vector<ShaderStageDesc> create_shader_stages() const;

        std::array<const Shader*, pipeline_size> shaders_;
        std::vector<ShaderStageDesc> shader_stages_;
    };

    // Shader handles for ShaderManager
    using shader_handle_key_t = std::uint16_t;
    ORION_DEFINE_HANDLE(ShaderHandle, shader_handle_key_t);

    // Shader paths for object types
    constexpr fs::path shader_object_base_path(ShaderObjectType object_type)
    {
        switch (object_type) {
            case ShaderObjectType::SpirV:
                return ORION_SPIRV_DIR;
            case ShaderObjectType::DXIL:
                return ORION_DXIL_DIR;
        }
        throw std::exception("invalid shader object type");
    }

    // Forward declare
    class RenderDevice;

    class ShaderManager
    {
    public:
        explicit ShaderManager(RenderDevice* device);
        explicit ShaderManager(RenderDevice* device, fs::path base_path);

        std::pair<ShaderHandle, const Shader*> load(const fs::path& filepath, std::string name = "");

        [[nodiscard]] std::pair<ShaderHandle, const Shader*> find(const std::string& name) const;
        [[nodiscard]] const Shader* get(ShaderHandle shader_handle) const;
        void remove(ShaderHandle shader_handle);

        [[nodiscard]] bool exists(ShaderHandle shader_handle) const;

        [[nodiscard]] ShaderEffect make_shader_effect(const std::string& vs_name, const std::string& ps_name) const;

    private:
        static spdlog::logger* logger();

        RenderDevice* device_;
        fs::path base_path_;
        std::unordered_map<ShaderHandle, Shader> shaders_;
    };
} // namespace orion
