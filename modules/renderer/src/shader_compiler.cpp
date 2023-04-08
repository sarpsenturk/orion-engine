#include "orion-renderer/shader_compiler.h"

#include "dxc_include.h"

#include <spdlog/sinks/stdout_color_sinks.h>

#ifndef FAILED
    #define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

namespace orion
{
    namespace detail
    {
        struct DxcInstance {
            CComPtr<IDxcUtils> utils;
            CComPtr<IDxcCompiler3> compiler;
            CComPtr<IDxcIncludeHandler> include_handler;
        };

        DxcInstance* dxc_create_instance()
        {
            CComPtr<IDxcUtils> utils;
            if (auto hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)); FAILED(hr)) {
                throw DxcInitError(hr);
            }
            SPDLOG_LOGGER_TRACE(ShaderCompiler::logger(), "Created IDxcUtils");
            CComPtr<IDxcCompiler3> compiler;
            if (auto hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)); FAILED(hr)) {
                throw DxcInitError(hr);
            }
            SPDLOG_LOGGER_TRACE(ShaderCompiler::logger(), "Created IDxcCompiler3");
            CComPtr<IDxcIncludeHandler> include_handler;
            if (auto hr = utils->CreateDefaultIncludeHandler(&include_handler); FAILED(hr)) {
                throw DxcInitError(hr);
            }
            SPDLOG_LOGGER_TRACE(ShaderCompiler::logger(), "Created IDxcIncludeHandler");
            SPDLOG_LOGGER_DEBUG(ShaderCompiler::logger(), "Created Dxc instance");
            return new DxcInstance(std::move(utils), std::move(compiler), std::move(include_handler));
        }

        void dxc_destroy_instance(DxcInstance* instance) { delete instance; }
    } // namespace detail

    DxcInitError::DxcInitError(long hresult)
        : hresult_(hresult)
    {
    }

    const std::shared_ptr<spdlog::logger> ShaderCompiler::s_logger = spdlog::stdout_color_mt("orion-shader-compiler");

    ShaderCompiler::ShaderCompiler()
        : dxc_instance_(detail::dxc_create_instance(), &detail::dxc_destroy_instance)
    {
    }
} // namespace orion
