#include "orion-renderer/shader.h"

#include "orion-renderapi/render_device.h"

#ifndef ORION_SHADER_MANAGER_LOG_LEVEL
    #define ORION_SHADER_MANAGER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    ShaderManager::ShaderManager(RenderDevice* device)
        : device_(device)
        , object_type_(device->shader_object_type())
    {
    }

    ShaderModuleHandle ShaderManager::create_shader_module(std::string_view source, const char* entry_point, ShaderStageFlags stage) const
    {
        const auto compile_result = shader_compiler_.compile({
            .source = source,
            .entry_point = entry_point,
            .stage = stage,
            .object_type = object_type_,
        });

        return device_->create_shader_module({
            .byte_code = compile_result.value().get_binary(),
        });
    }

    ShaderModuleHandle ShaderManager::create_vs(std::string_view source) const
    {
        return create_shader_module(source, vs_entry_point, ShaderStageFlags::Vertex);
    }

    ShaderModuleHandle ShaderManager::create_fs(std::string_view source) const
    {
        return create_shader_module(source, fs_entry_point, ShaderStageFlags::Fragment);
    }
} // namespace orion
