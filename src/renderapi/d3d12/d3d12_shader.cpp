#include "d3d12_shader.h"

#include "orion_dx12.h"

#include "orion/assertion.h"

#include <spdlog/spdlog.h>

#include <array>

namespace orion
{
    namespace
    {
        const wchar_t* shader_target(ShaderType shader_type)
        {
            switch (shader_type) {
                case ShaderType::Vertex:
                    return L"vs_6_0";
                case ShaderType::Pixel:
                    return L"ps_6_0";
            }
            unreachable();
        }
    } // namespace

    D3D12ShaderCompiler::D3D12ShaderCompiler()
    {
        hr_assert(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler_)));
        hr_assert(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils_)));
        hr_assert(dxc_utils_->CreateDefaultIncludeHandler(&dxc_include_handler_));
        SPDLOG_TRACE("Initialized DXC");
    }

    std::vector<std::byte> D3D12ShaderCompiler::compile_api(const ShaderCompileOptions& options)
    {
        auto args = std::array{
            L"-E", L"main",                     // Entry point
            L"-T", shader_target(options.type), // Target
        };

        const auto buffer = DxcBuffer{
            .Ptr = options.source.data(),
            .Size = options.source.size(),
            .Encoding = DXC_CP_ACP,
        };

        CComPtr<IDxcResult> results;
        hr_assert(dxc_compiler_->Compile(&buffer, args.data(), static_cast<UINT32>(args.size()), dxc_include_handler_, IID_PPV_ARGS(&results)));

        CComPtr<IDxcBlobUtf8> errors;
        hr_assert(results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
        if (errors->GetStringLength() != 0) {
            SPDLOG_ERROR("DirectXShaderCompiler error: {}", errors->GetStringPointer());
        }

        HRESULT status;
        hr_assert(results->GetStatus(&status));
        if (FAILED(status)) {
            SPDLOG_ERROR("Shader compilation failed");
            return {};
        }

        CComPtr<IDxcBlob> binary;
        hr_assert(results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&binary), nullptr));
        SPDLOG_TRACE("DirectXShaderCompiler: shader compiled successfully");

        std::vector<std::byte> output(binary->GetBufferSize());
        std::memcpy(output.data(), binary->GetBufferPointer(), binary->GetBufferSize());
        return output;
    }
} // namespace orion
