#include "orion-shader-compiler/compiler.h"

#include "dxc_impl.h"

#include "orion-utils/string.h"

#ifndef ORION_SHADER_COMPILER_LOG_LEVEL
    #define ORION_SHADER_COMPILER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cstring>
#include <string>
#include <vector>

namespace orion
{
    namespace
    {
        spdlog::logger* logger()
        {
            static const auto dxc_logger = []() {
                auto logger = spdlog::stdout_color_st("orion-shader-compiler");
                logger->set_pattern("[%n] [%^%l%$] %v");
                logger->set_level((static_cast<spdlog::level::level_enum>(ORION_SHADER_COMPILER_LOG_LEVEL)));
                return logger;
            }();
            return dxc_logger.get();
        }

        // Add SPIR-V specific compiler flags to out_args
        void add_spirv_flags(std::vector<LPCWSTR>& out_args, const ShaderCompileDesc& compile_desc)
        {
            // Emit SPIR-V
            out_args.push_back(L"-spirv");

            // Invert Y-axis if vertex, geometry or hull shader
            if (const auto profile = std::string_view{compile_desc.target_profile};
                is_vertex_shader(profile) || is_geometry_shader(profile) || is_hull_shader(profile)) {
                out_args.push_back(L"-fvk-invert-y");
            }
        }

        // Add DXIL specific compiler flags to out_args
        void add_dxil_flags(std::vector<LPCWSTR>& /*unused*/, const ShaderCompileDesc& /*unused*/)
        {
        }

        // Extract DXC_OUT_OBJECT into vector<char>
        std::vector<char> get_object_blob(IDxcResult* result)
        {
            CComPtr<IDxcBlob> out_blob;
            ORION_DXC_ASSERT(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&out_blob), nullptr));
            std::vector<char> out_object(out_blob->GetBufferSize());
            std::memcpy(out_object.data(), out_blob->GetBufferPointer(), out_blob->GetBufferSize());
            return out_object;
        }

        ShaderCompileResult compile_blob(IDxcBlobEncoding* blob, const ShaderCompileDesc& desc)
        {
            std::vector<LPCWSTR> arguments;

            const auto wfilename = string_to_wstring(desc.filename);
            arguments.push_back(wfilename.c_str());

            const auto wentry = string_to_wstring(desc.entry_point);
            arguments.insert(arguments.end(), {L"-E", wentry.c_str()});

            const auto wtarget = string_to_wstring(desc.target_profile);
            arguments.insert(arguments.end(), {L"-T", wtarget.c_str()});

            switch (desc.shader_il) {
                case ShaderIL::SpirV:
                    add_spirv_flags(arguments, desc);
                    break;
                case ShaderIL::DXIL:
                    add_dxil_flags(arguments, desc);
                    break;
            }

            const auto source_buffer = DxcBuffer{
                .Ptr = blob->GetBufferPointer(),
                .Size = blob->GetBufferSize(),
                .Encoding = DXC_CP_ACP,
            };

            auto* compiler = dxc_instance()->compiler();
            CComPtr<IDxcResult> compile_result;
            ORION_DXC_ASSERT(
                compiler->Compile(
                    &source_buffer,
                    arguments.data(),
                    static_cast<UINT32>(arguments.size()),
                    dxc_instance()->include_handler(),
                    IID_PPV_ARGS(&compile_result)));

            HRESULT compile_status{};
            ORION_DXC_ASSERT(compile_result->GetStatus(&compile_status));

            CComPtr<IDxcBlobUtf8> errors;
            ORION_DXC_ASSERT(compile_result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));

            if (errors != nullptr && errors->GetStringLength() != 0) {
                if (FAILED(compile_status)) {
                    SPDLOG_LOGGER_ERROR(logger(), "Shader compilation failed:\n{}", errors->GetStringPointer());
                    return make_unexpected(ShaderCompileFail{.message = std::string{errors->GetStringPointer(), errors->GetBufferSize()}});
                }
                SPDLOG_LOGGER_WARN(logger(), "Shader compiled with warnings:\n{}", errors->GetStringPointer());
            }

            return ShaderCompileSuccess{
                .object = get_object_blob(compile_result.p),
            };
        }
    } // namespace

    bool is_vertex_shader(std::string_view profile)
    {
        return profile.starts_with("vs_");
    }

    bool is_pixel_shader(std::string_view profile)
    {
        return profile.starts_with("ps_");
    }

    bool is_geometry_shader(std::string_view profile)
    {
        return profile.starts_with("gs_");
    }

    bool is_hull_shader(std::string_view profile)
    {
        return profile.starts_with("hs_");
    }

    bool is_compute_shader(std::string_view profile)
    {
        return profile.starts_with("cs_");
    }

    namespace shader_compiler
    {
        ShaderCompileResult compile_string(const std::string& source, const ShaderCompileDesc& desc)
        {
            auto* utils = dxc_instance()->utils();
            CComPtr<IDxcBlobEncoding> source_blob;
            ORION_DXC_ASSERT(utils->CreateBlob(source.data(), static_cast<UINT32>(source.size()), 0, &source_blob));
            return compile_blob(source_blob.p, desc);
        }

        ShaderCompileResult compile_file(const std::string& filename, const ShaderCompileDesc& desc)
        {
            auto* utils = dxc_instance()->utils();
            CComPtr<IDxcBlobEncoding> source_blob;
            const auto wfilename = string_to_wstring(filename);
            ORION_DXC_ASSERT(utils->LoadFile(wfilename.c_str(), nullptr, &source_blob));
            return compile_blob(source_blob.p, desc);
        }
    } // namespace shader_compiler
} // namespace orion
