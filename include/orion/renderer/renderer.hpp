#pragma once

#include <tl/expected.hpp>

#include <memory>
#include <string>

namespace orion
{
    class Renderer
    {
    public:
        static tl::expected<Renderer, std::string> initialize();
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) noexcept;
        Renderer& operator=(Renderer&&) noexcept;
        ~Renderer();

    private:
        struct Impl;
        explicit Renderer(std::unique_ptr<Impl> impl);
        std::unique_ptr<Impl> impl_;
    };
} // namespace orion
