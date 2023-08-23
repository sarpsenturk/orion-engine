#include "orion-renderer/shader.h"

#include "orion-utils/bitflag.h"

#ifndef ORION_SHADER_MANAGER_LOG_LEVEL
    #define ORION_SHADER_MANAGER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

#include <functional>

namespace orion
{
    Shader::Shader(UniqueShaderModule vertex_shader, UniqueShaderModule fragment_shader)
        : vertex_shader_(std::move(vertex_shader))
        , fragment_shader_(std::move(fragment_shader))
        , pipeline_shader_desc_(make_pipeline_desc())
    {
    }

    std::vector<ShaderStageDesc> Shader::make_pipeline_desc() const
    {
        std::vector<ShaderStageDesc> pipeline_shader_stages;
        if (vertex_shader_) {
            pipeline_shader_stages.push_back({
                .module = vertex_shader_.get(),
                .stage = ShaderStageFlags::Vertex,
                .entry_point = vs_entry_point,
            });
        }
        if (fragment_shader_) {
            pipeline_shader_stages.push_back({
                .module = fragment_shader_.get(),
                .stage = ShaderStageFlags::Fragment,
                .entry_point = fs_entry_point,
            });
        }
        return pipeline_shader_stages;
    }

    spdlog::logger* ShaderManager::logger()
    {
        static const auto logger = create_logger("orion-shader", ORION_SHADER_MANAGER_LOG_LEVEL);
        return logger.get();
    }

    ShaderManager::ShaderManager(RenderDevice* device)
        : device_(device)
        , object_type_(device->shader_object_type())
    {
    }

    std::pair<ShaderHandle, const Shader*> ShaderManager::add_from_source(const std::string& source, ShaderStageFlags shader_stages)
    {
        return add(source, shader_stages, &ShaderCompiler::compile_from_source);
    }

    std::pair<ShaderHandle, const Shader*> ShaderManager::add_from_file(const std::string& filename, ShaderStageFlags shader_stages)
    {
        return add(filename, shader_stages, &ShaderCompiler::compile_from_file);
    }

    void ShaderManager::remove(ShaderHandle shader_handle) noexcept
    {
        if (const auto erased = shaders_.erase(shader_handle); erased == 0) {
            SPDLOG_LOGGER_WARN(logger(), "Trying to remove {} which doesn't exist", shader_handle);
            return;
        }
        SPDLOG_LOGGER_DEBUG(logger(), "Removed shader {}", shader_handle);
    }

    std::pair<ShaderHandle, const Shader*> ShaderManager::add(const std::string& source_str, ShaderStageFlags shader_stages, shader_compiler_compile_fn compile_fn)
    {
        auto create_shader_module = [&source_str, compile_fn, this](ShaderStageFlags stage, const char* entry_point) {
            const auto compile_desc = ShaderCompileDesc{
                .entry_point = entry_point,
                .shader_stage = stage,
                .object_type = object_type_,
            };
            const auto compile_result = std::invoke(compile_fn, shader_compiler_, source_str, compile_desc);
            if (!compile_result) {
                SPDLOG_LOGGER_ERROR(logger(), "Failed to compile shader: {}", compile_result.error());
                return UniqueShaderModule{nullptr};
            }
            const auto module_desc = ShaderModuleDesc{.byte_code = compile_result->binary};
            return device_->make_unique(ShaderModuleHandle_tag{}, module_desc);
        };

        using enum ShaderStageFlags;
        UniqueShaderModule vertex_shader = !!(shader_stages & Vertex) ? create_shader_module(Vertex, vs_entry_point) : nullptr;
        UniqueShaderModule fragment_shader = !!(shader_stages & Fragment) ? create_shader_module(Fragment, fs_entry_point) : nullptr;

        // Insert new shader
        const auto handle = next_handle();
        auto [iter, inserted] = shaders_.emplace(handle, Shader{std::move(vertex_shader), std::move(fragment_shader)});
        ORION_ENSURES(inserted);

        const auto& shader = iter->second;
        SPDLOG_LOGGER_DEBUG(logger(), "Created {} with stage(s) {}", handle, shader_stages);
        return std::make_pair(handle, &shader);
    }
} // namespace orion
