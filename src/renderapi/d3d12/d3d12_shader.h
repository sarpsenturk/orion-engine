#pragma once

#include "orion/renderapi/shader.h"

#include <atlbase.h>
#include <dxcapi.h>

namespace orion
{
    class D3D12ShaderCompiler final : public ShaderCompiler
    {
    public:
        D3D12ShaderCompiler();

    private:
        std::vector<std::byte> compile_api(const ShaderCompileOptions& options) override;

        CComPtr<IDxcCompiler3> dxc_compiler_;
        CComPtr<IDxcUtils> dxc_utils_;
        CComPtr<IDxcIncludeHandler> dxc_include_handler_;
    };
} // namespace orion
