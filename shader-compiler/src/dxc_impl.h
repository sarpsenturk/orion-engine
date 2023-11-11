#pragma once

#ifdef ORION_PLATFORM_WINDOWS
    #include <atlbase.h>
#else
    #error "Orion Shader Compiler is not currently supported on this platform"
#endif

#include <dxcapi.h>

#include "orion-utils/assertion.h"
#define ORION_DXC_ASSERT(hresult) ORION_ASSERT(SUCCEEDED(hresult))

namespace orion
{
    class DxcInstance
    {
    public:
        explicit DxcInstance(CComPtr<IDxcIncludeHandler> include_handler = nullptr);

        [[nodiscard]] IDxcCompiler3* compiler() const noexcept { return compiler_.p; }
        [[nodiscard]] IDxcUtils* utils() const noexcept { return utils_.p; }
        [[nodiscard]] IDxcIncludeHandler* include_handler() const noexcept { return include_handler_.p; }

    private:
        CComPtr<IDxcCompiler3> compiler_;
        CComPtr<IDxcUtils> utils_;
        CComPtr<IDxcIncludeHandler> include_handler_;
    };

    // Return a thread_local instance
    [[nodiscard]] DxcInstance* dxc_instance();
} // namespace orion
