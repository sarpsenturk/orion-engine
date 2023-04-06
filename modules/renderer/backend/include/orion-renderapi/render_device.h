#pragma once

#include "handles.h"
#include "orion-renderapi/types.h"
#include "render_context.h"
#include "render_pass.h"
#include "swapchain.h"

#include <orion-core/window.h> // orion::Window

namespace orion
{
    class RenderDevice
    {
    public:
        RenderDevice() = default;
        virtual ~RenderDevice() = default;

        [[nodiscard]] virtual std::unique_ptr<RenderContext> create_render_context() = 0;

        [[nodiscard]] Swapchain create_swapchain(const Window& window, SwapchainDesc desc);
        [[nodiscard]] RenderPass create_render_pass(const RenderPassDesc& desc);

    protected:
        RenderDevice(const RenderDevice&) = default;
        RenderDevice(RenderDevice&&) noexcept = default;
        RenderDevice& operator=(const RenderDevice&) = default;
        RenderDevice& operator=(RenderDevice&&) noexcept = default;

    private:
        template<typename HandleType>
        auto make_handle_ref(HandleType value)
        {
            return std::shared_ptr<HandleType>(new HandleType(value), [this](HandleType* handle) { destroy(*handle); });
        }

        [[nodiscard]] virtual SwapchainHandle create_swapchain_api(const Window& window, const SwapchainDesc& desc, SwapchainHandle existing) = 0;
        [[nodiscard]] virtual RenderPassHandle create_render_pass_api(const RenderPassDesc& desc, RenderPassHandle existing) = 0;

        virtual void destroy(SwapchainHandle swapchain_handle) = 0;
        virtual void destroy(RenderPassHandle render_pass_handle) = 0;
    };
} // namespace orion
