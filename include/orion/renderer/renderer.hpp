#pragma once

#include <tl/expected.hpp>

#include <memory>
#include <string>

namespace orion
{
    struct RendererDesc {
        const class Window& window;
    };

    class Renderer
    {
    public:
        static tl::expected<Renderer, std::string> initialize(const RendererDesc& desc);
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) noexcept;
        Renderer& operator=(Renderer&&) noexcept;
        ~Renderer();

        void new_frame();
        void render();

        [[nodiscard]] bool swapchain_out_of_date() const noexcept;
        tl::expected<void, std::string> recreate_swapchain(int width, int height);

    private:
        struct Impl;
        explicit Renderer(std::unique_ptr<Impl> impl);
        std::unique_ptr<Impl> impl_;
    };
} // namespace orion
