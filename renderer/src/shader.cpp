#include "orion-renderer/shader.h"

#include "orion-renderapi/render_device.h"

#ifndef ORION_SHADER_MANAGER_LOG_LEVEL
    #define ORION_SHADER_MANAGER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <fmt/std.h>
#include <spdlog/spdlog.h>

#include <algorithm>

namespace orion
{
    spdlog::logger* ShaderManager::logger()
    {
        static const auto logger = create_logger("orion-shader-manager", ORION_SHADER_MANAGER_LOG_LEVEL);
        return logger.get();
    }

    Shader::Shader(std::string name, ShaderModuleHandle shader_module)
        : name_(std::move(name))
        , shader_module_(shader_module)
    {
    }

    ShaderEffect::ShaderEffect(std::array<const Shader*, pipeline_size> shaders)
        : shaders_(shaders)
        , shader_stages_(create_shader_stages())
    {
    }

    std::vector<ShaderStageDesc> ShaderEffect::create_shader_stages() const
    {
        std::vector<ShaderStageDesc> shader_stages;
        auto add_stage = [&](const Shader* shader, ShaderStageFlags stage, const char* entry_point) {
            if (shader != nullptr) {
                shader_stages.push_back({.module = shader->shader_module(), .stage = stage, .entry_point = entry_point});
            }
        };

        add_stage(vertex_shader(), ShaderStageFlags::Vertex, ORION_VS_ENTRY);
        add_stage(pixel_shader(), ShaderStageFlags::Pixel, ORION_PS_ENTRY);
        return shader_stages;
    }

    ShaderManager::ShaderManager(RenderDevice* device)
        : device_(device)
        , base_path_(shader_object_base_path(device->shader_object_type()))
    {
        SPDLOG_LOGGER_TRACE(logger(), "ShaderManager initialized. Base path={}", base_path_);
    }

    ShaderManager::ShaderManager(RenderDevice* device, FilePath base_path)
        : device_(device)
        , base_path_(std::move(base_path))
    {
        SPDLOG_LOGGER_TRACE(logger(), "ShaderManager initialized. Base path={}", base_path_);
    }

    std::pair<ShaderHandle, const Shader*> ShaderManager::load(const FilePath& filepath, std::string name)
    {
        auto file = binary_input_file(base_path_ / filepath);
        auto object = file.read_all();
        auto shader_module = device_->create_shader_module({.byte_code = object});
        auto handle = static_cast<shader_handle_key_t>(shaders_.size());

        if (name.empty()) {
            name = filepath.string();
        }
        auto [iterator, inserted] = shaders_.insert(std::make_pair(handle, Shader{std::move(name), shader_module}));
        ORION_ENSURES(inserted);
        return std::make_pair(iterator->first, &(iterator->second));
    }

    std::pair<ShaderHandle, const Shader*> ShaderManager::find(const std::string& name) const
    {
        if (name.empty()) {
            return std::make_pair(ShaderHandle::invalid(), nullptr);
        }

        auto find_by_name = [&](const auto& shader) { return shader.second.name() == name; };
        if (auto iter = std::find_if(shaders_.begin(), shaders_.end(), find_by_name);
            iter != shaders_.end()) {
            return std::make_pair(iter->first, &(iter->second));
        }
        SPDLOG_LOGGER_WARN(logger(), "No shader with name '{}' found!", name);
        return std::make_pair(ShaderHandle::invalid(), nullptr);
    }

    const Shader* ShaderManager::get(ShaderHandle shader_handle) const
    {
        if (auto iter = shaders_.find(shader_handle); iter != shaders_.end()) {
            return &(iter->second);
        }
        SPDLOG_LOGGER_WARN(logger(), "No shader with handle {} found!", shader_handle);
        return nullptr;
    }

    void ShaderManager::remove(ShaderHandle shader_handle)
    {
        if (shaders_.erase(shader_handle) == 0) {
            SPDLOG_LOGGER_WARN(logger(), "No shader with handle {}. Nothing removed", shader_handle);
        }
    }

    bool ShaderManager::exists(ShaderHandle shader_handle) const
    {
        return shaders_.contains(shader_handle);
    }

    ShaderEffect ShaderManager::make_shader_effect(const std::string& vs_name, const std::string& ps_name) const
    {
        const Shader* vertex_shader = find(vs_name).second;
        const Shader* pixel_shader = find(ps_name).second;
        return ShaderEffect{std::array{vertex_shader, pixel_shader}};
    }

    ShaderEffect ShaderManager::load_shader_effect(const std::string& name)
    {
        return ShaderEffect{std::array{load(name + ".vs").second, load(name + ".ps").second}};
    }
} // namespace orion
