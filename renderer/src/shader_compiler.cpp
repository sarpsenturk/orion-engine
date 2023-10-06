#include "orion-renderer/shader_compiler.h"

#include "dxc_include.h"
#include <d3d12shader.h> // Shader reflection

#ifndef ORION_SHADER_COMPILER_LOG_LEVEL
    #define ORION_SHADER_COMPILER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

#include "orion-utils/static_vector.h"

#include <codecvt>
#include <locale>

#include <cstring>

namespace orion
{
    namespace
    {
        spdlog::logger* logger()
        {
            static const auto dxc_logger = create_logger("orion-shader-compiler", ORION_SHADER_COMPILER_LOG_LEVEL);
            return dxc_logger.get();
        }

        // Returns the DXC shader profile appropriate for the shader type
        const wchar_t* to_shader_profile(ShaderStageFlags shader_type) noexcept
        {
            switch (shader_type) {
                case ShaderStageFlags::Vertex:
                    return L"vs_6_0";
                case ShaderStageFlags::Fragment:
                    return L"ps_6_0";
            }
            ORION_ASSERT(!"Invalid shader type");
            return nullptr;
        }

        auto to_wstring(const char* string)
        {
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4996)
#endif
            static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.from_bytes(string);
#ifdef _MSC_VER
    #pragma warning(pop)
#endif
        }

        void dxc_assert(HRESULT hresult)
        {
#ifdef ORION_BUILD_DEBUG
            if (FAILED(hresult)) {
                SPDLOG_LOGGER_CRITICAL(logger(), "Dxc assertion failed");
                ORION_ASSERT(false);
            }
#endif
        }
    } // namespace

    struct ShaderObject::ShaderObjectImpl {
        ComPtr<IDxcResult> compile_result;

        // Forwards call to IDxcResult::GetOutput
        HRESULT GetOutput(DXC_OUT_KIND dxcOutKind, const _GUID& iid, void** ppvObject, IDxcBlobWide** ppOutputName)
        {
            return compile_result->GetOutput(dxcOutKind, iid, ppvObject, ppOutputName);
        }
    };

    ShaderObject::ShaderObject(std::unique_ptr<ShaderObjectImpl> data)
        : impl_(std::move(data))
    {
    }

    ShaderObject::ShaderObject(ShaderObject&&) noexcept = default;

    ShaderObject& ShaderObject::operator=(ShaderObject&&) noexcept = default;

    ShaderObject::~ShaderObject() = default;

    std::vector<std::byte> ShaderObject::get_binary() const
    {
        ComPtr<IDxcBlob> binary_output;
        dxc_assert(impl_->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&binary_output), nullptr));
        const auto binary_size = binary_output->GetBufferSize();
        SPDLOG_LOGGER_TRACE(logger(), "Shader binary {} bytes", binary_size);
        std::vector<std::byte> binary(binary_size);
        std::memcpy(binary.data(), binary_output->GetBufferPointer(), binary_size);
        return binary;
    }

    // DirectXShaderCompiler instance
    struct ShaderCompiler::ShaderCompilerImpl {
        ComPtr<IDxcCompiler3> compiler;
        ComPtr<IDxcUtils> utils;
        ComPtr<IDxcIncludeHandler> include_handler;

        ShaderCompilerImpl()
        {
            dxc_assert(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
            dxc_assert(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
            dxc_assert(utils->CreateDefaultIncludeHandler(&include_handler));
            SPDLOG_LOGGER_TRACE(logger(), "DirectXShaderCompiler instance created");
        }
    };

    ShaderCompiler::ShaderCompiler()
        : impl_(std::make_unique<ShaderCompilerImpl>())
    {
    }

    ShaderCompiler::ShaderCompiler(ShaderCompiler&&) noexcept = default;

    ShaderCompiler& ShaderCompiler::operator=(ShaderCompiler&&) noexcept = default;

    ShaderCompiler::~ShaderCompiler() = default;

    expected<ShaderObject, ShaderCompileFail> ShaderCompiler::compile(const ShaderCompileDesc& desc)
    {
        SPDLOG_LOGGER_DEBUG(logger(), "Compiling {} shader...", desc.stage);
        auto* compiler = impl_->compiler.Get();
        auto* utils = impl_->utils.Get();
        auto* include_handler = impl_->include_handler.Get();

        ComPtr<IDxcBlobEncoding> source_blob;
        dxc_assert(utils->CreateBlob(desc.source.data(), static_cast<UINT>(desc.source.size()), 0, &source_blob));

        const auto source_buffer = DxcBuffer{
            .Ptr = source_blob->GetBufferPointer(),
            .Size = source_blob->GetBufferSize(),
            .Encoding = DXC_CP_ACP,
        };

        constexpr auto max_arguments = 32;
        static_vector<const wchar_t*, max_arguments> arguments;

        const auto entry_point = to_wstring(desc.entry_point);
        arguments.insert(arguments.end(), {L"-E", entry_point.data()});

        const wchar_t* shader_target = to_shader_profile(desc.stage);
        arguments.insert(arguments.end(), {L"-T", shader_target});

        // This isn't maintainable.
        // We need to figure out a better way to separate code specific to SPIR-V or DXIL codegen
        if (desc.object_type == ShaderObjectType::SpirV) {
            arguments.push_back(L"-spirv");
            if (desc.stage == ShaderStageFlags::Vertex) {
                arguments.push_back(L"-fvk-invert-y");
            }
        }

        ComPtr<IDxcResult> result;
        dxc_assert(compiler->Compile(&source_buffer, arguments.data(), arguments.size(), include_handler, IID_PPV_ARGS(&result)));

        HRESULT compile_status;
        dxc_assert(result->GetStatus(&compile_status));

        ComPtr<IDxcBlobUtf8> errors;
        dxc_assert(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));

        if (errors != nullptr && errors->GetStringLength() != 0) {
            if (FAILED(compile_status)) {
                SPDLOG_LOGGER_ERROR(logger(), "Shader compilation failed:\n{}", errors->GetStringPointer());
                return make_unexpected(ShaderCompileFail{.error = {}, .message = "Compilation failed"});
            }
            SPDLOG_LOGGER_WARN(logger(), "Shader compilation succeeded with warnings:\n{}", errors->GetStringPointer());
        }

        return ShaderObject{std::make_unique<ShaderObject::ShaderObjectImpl>(std::move(result))};
    }
} // namespace orion
