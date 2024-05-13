#include "vulkan_reflection.h"

#include <spirv_reflect.h>

#include <algorithm>
#include <vector>

#define ORION_SPV_REFLECT_CHECK(expr)                              \
    do {                                                           \
        if (const auto result = expr) {                            \
            return make_unexpected(map_spv_reflect_error(result)); \
        }                                                          \
    } while (0)

namespace orion::vulkan
{
    namespace
    {
        ShaderReflectionError map_spv_reflect_error(SpvReflectResult result)
        {
            return ShaderReflectionError::InternalError;
        }

        expected<std::vector<SpvReflectInterfaceVariable*>, ShaderReflectionError> get_input_variables(const spv_reflect::ShaderModule& module)
        {
            uint32_t count = 0;
            ORION_SPV_REFLECT_CHECK(module.EnumerateInputVariables(&count, nullptr));
            std::vector<SpvReflectInterfaceVariable*> input_variables(count);
            ORION_SPV_REFLECT_CHECK(module.EnumerateInputVariables(&count, input_variables.data()));
            return input_variables;
        }

        expected<std::vector<SpvReflectDescriptorSet*>, ShaderReflectionError> get_descriptor_sets(const spv_reflect::ShaderModule& module)
        {
            uint32_t count = 0;
            ORION_SPV_REFLECT_CHECK(module.EnumerateDescriptorSets(&count, nullptr));
            std::vector<SpvReflectDescriptorSet*> descriptor_sets(count);
            ORION_SPV_REFLECT_CHECK(module.EnumerateDescriptorSets(&count, descriptor_sets.data()));
            return descriptor_sets;
        }

        expected<std::vector<SpvReflectBlockVariable*>, ShaderReflectionError> get_push_constants(const spv_reflect::ShaderModule& module)
        {
            uint32_t count = 0;
            ORION_SPV_REFLECT_CHECK(module.EnumeratePushConstantBlocks(&count, nullptr));
            std::vector<SpvReflectBlockVariable*> push_constants(count);
            ORION_SPV_REFLECT_CHECK(module.EnumeratePushConstantBlocks(&count, push_constants.data()));
            return push_constants;
        }

        Format map_spv_format(SpvReflectFormat format)
        {
            switch (format) {
                case SPV_REFLECT_FORMAT_UNDEFINED:
                    break;
                case SPV_REFLECT_FORMAT_R32_UINT:
                    return Format::R32_Uint;
                case SPV_REFLECT_FORMAT_R32_SINT:
                    return Format::R32_Int;
                case SPV_REFLECT_FORMAT_R32_SFLOAT:
                    return Format::R32_Float;
                case SPV_REFLECT_FORMAT_R32G32_UINT:
                    return Format::R32G32B32_Uint;
                case SPV_REFLECT_FORMAT_R32G32_SINT:
                    return Format::R32G32_Int;
                case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
                    return Format::R32G32_Float;
                case SPV_REFLECT_FORMAT_R32G32B32_UINT:
                    return Format::R32G32B32_Uint;
                case SPV_REFLECT_FORMAT_R32G32B32_SINT:
                    return Format::R32G32B32_Int;
                case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
                    return Format::R32G32B32_Float;
                case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
                    return Format::R32G32B32A32_Uint;
                case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
                    return Format::R32G32B32A32_Int;
                case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
                    return Format::R32G32B32A32_Float;
                default:
                    break;
            }
            return Format::Undefined;
        }

        const char* get_spv_builtin_name(SpvBuiltIn builtin)
        {
            switch (builtin) {
                case SpvBuiltInVertexIndex:
                    return shader_builtins::vertex_index;
                default:
                    break;
            }
            return shader_builtins::unknown;
        }

        DescriptorType map_spv_descriptor_type(SpvReflectDescriptorType type)
        {
            switch (type) {
                case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
                    return DescriptorType::Sampler;
                case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    return DescriptorType::SampledImage;
                case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    return DescriptorType::ConstantBuffer;
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    return DescriptorType::StorageBuffer;
                default:
                    break;
            }
            ORION_ASSERT(false);
            return {};
        }

        ShaderStageFlags map_spv_shader_stage(SpvReflectShaderStageFlagBits shader_stages)
        {
            ShaderStageFlags result = {};
            if (shader_stages & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
                result |= ShaderStageFlags::Vertex;
            }
            if (shader_stages & SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT) {
                result |= ShaderStageFlags::Pixel;
            }
            return result;
        }

        std::vector<ShaderReflectionInputVariable> map_input_variables(const std::vector<SpvReflectInterfaceVariable*>& input_variables)
        {
            std::vector<ShaderReflectionInputVariable> result(input_variables.size());
            std::ranges::transform(input_variables, result.begin(), [](const SpvReflectInterfaceVariable* input_variable) {
                const auto is_builtin = input_variable->built_in != -1;
                return ShaderReflectionInputVariable{
                    .name = is_builtin ? get_spv_builtin_name(input_variable->built_in) : input_variable->name,
                    .format = map_spv_format(input_variable->format),
                    .builtin = is_builtin,
                };
            });
            return result;
        }

        std::vector<ShaderReflectionDescriptorBinding> map_descriptor_bindings(const SpvReflectDescriptorSet* descriptor_set)
        {
            std::vector<ShaderReflectionDescriptorBinding> bindings(descriptor_set->binding_count);
            std::transform(descriptor_set->bindings, descriptor_set->bindings + descriptor_set->binding_count, bindings.begin(), [](const SpvReflectDescriptorBinding* binding) {
                return ShaderReflectionDescriptorBinding{
                    .binding = binding->binding,
                    .name = binding->name,
                    .type = map_spv_descriptor_type(binding->descriptor_type),
                    .count = binding->count,
                };
            });
            return bindings;
        }

        std::vector<ShaderReflectionDescriptorSet> map_descriptor_sets(const std::vector<SpvReflectDescriptorSet*>& descriptor_sets)
        {
            std::vector<ShaderReflectionDescriptorSet> result(descriptor_sets.size());
            std::ranges::transform(descriptor_sets, result.begin(), [](const SpvReflectDescriptorSet* descriptor_set) {
                return ShaderReflectionDescriptorSet{
                    .set = descriptor_set->set,
                    .bindings = map_descriptor_bindings(descriptor_set),
                };
            });
            return result;
        }

        std::vector<ShaderReflectionPushConstant> map_push_constants(const std::vector<SpvReflectBlockVariable*>& push_constants)
        {
            std::vector<ShaderReflectionPushConstant> result(push_constants.size());
            std::ranges::transform(push_constants, result.begin(), [](const SpvReflectBlockVariable* push_constant) {
                return ShaderReflectionPushConstant{
                    .size = push_constant->size,
                };
            });
            return result;
        }
    } // namespace

    ShaderReflectionResult VulkanShaderReflector::reflect_api(std::span<const std::byte> shader_code)
    {
        auto module = spv_reflect::ShaderModule{shader_code.size(), shader_code.data(), SPV_REFLECT_MODULE_FLAG_NO_COPY};

        const auto input_variables = get_input_variables(module);
        if (!input_variables) {
            return make_unexpected(input_variables.error());
        }

        const auto descriptor_sets = get_descriptor_sets(module);
        if (!descriptor_sets) {
            return make_unexpected(descriptor_sets.error());
        }

        const auto push_constants = get_push_constants(module);
        if (!push_constants) {
            return make_unexpected(push_constants.error());
        }

        return ShaderReflection{
            .entry_point = module.GetEntryPointName(0),
            .stage = map_spv_shader_stage(module.GetShaderStage()),
            .input_variables = map_input_variables(*input_variables),
            .descriptors = map_descriptor_sets(*descriptor_sets),
            .push_constants = map_push_constants(*push_constants),
        };
    }
} // namespace orion::vulkan
