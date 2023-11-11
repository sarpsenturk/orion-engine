#include "dxc_impl.h"

namespace orion
{
    DxcInstance::DxcInstance(CComPtr<IDxcIncludeHandler> include_handler)
    {
        ORION_DXC_ASSERT(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler_)));
        ORION_DXC_ASSERT(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils_)));
        if (include_handler == nullptr) {
            ORION_DXC_ASSERT(utils_->CreateDefaultIncludeHandler(&include_handler_));
        } else {
            include_handler_ = std::move(include_handler);
        }
    }

    DxcInstance* dxc_instance()
    {
        thread_local static auto instance = DxcInstance{};
        return &instance;
    }
} // namespace orion
