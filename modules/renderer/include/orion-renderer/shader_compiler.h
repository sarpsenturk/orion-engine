#pragma once

#include <memory>                 // std::unique_ptr
#include <orion-core/exception.h> // orion::Exception
#include <spdlog/spdlog.h>        // SPDLOG_*

namespace orion
{
    class DxcInitError : public OrionException
    {
    public:
        explicit DxcInitError(long hresult);

        [[nodiscard]] const char* type() const noexcept override { return "DxcInitError"; }
        [[nodiscard]] int return_code() const noexcept override { return hresult_; }

    private:
        long hresult_;
    };

    namespace detail
    {
        struct DxcInstance;
        DxcInstance* dxc_create_instance();
        void dxc_destroy_instance(DxcInstance* instance);
        using DxcInstancePtr = std::unique_ptr<DxcInstance, decltype(&dxc_destroy_instance)>;
    } // namespace detail

    class ShaderCompiler
    {
    public:
        ShaderCompiler();

        static auto logger() { return s_logger.get(); }

    private:
        detail::DxcInstancePtr dxc_instance_;

        static const std::shared_ptr<spdlog::logger> s_logger;
    };
} // namespace orion
