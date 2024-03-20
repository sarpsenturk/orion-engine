#pragma once

namespace orion
{
    class ImGuiContext
    {
    public:
        ImGuiContext() = default;
        ImGuiContext(const ImGuiContext&) = delete;
        ImGuiContext(ImGuiContext&& other) noexcept;
        ImGuiContext& operator=(const ImGuiContext&) = delete;
        ImGuiContext& operator=(ImGuiContext&&) = delete;
        ~ImGuiContext();

    private:
        bool destroy_ = true;
    };
} // namespace orion
