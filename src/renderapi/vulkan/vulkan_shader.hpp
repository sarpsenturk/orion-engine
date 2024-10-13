#pragma once

#include "orion/renderapi/shader.hpp"

#ifdef _WIN32
    #include <atlbase.h>
#else
    #include <dxc/WinAdapter.h>
#endif
#include <dxc/dxcapi.h>

namespace orion
{
    class VulkanShaderCompiler final : public ShaderCompiler
    {
    public:
        VulkanShaderCompiler();

    private:
        std::vector<std::byte> compile_api(const ShaderCompileOptions& options) override;

        CComPtr<IDxcCompiler3> dxc_compiler_;
        CComPtr<IDxcUtils> dxc_utils_;
        CComPtr<IDxcIncludeHandler> dxc_include_handler_;
    };
} // namespace orion
