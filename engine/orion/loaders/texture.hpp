#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <vector>

namespace orion
{
    struct LoadImageResult {
        std::uint32_t width = 0u;
        std::uint32_t height = 0u;
        // Owned, tightly packed RGBA8 pixels in source row order. No color-space conversion.
        std::vector<std::uint8_t> pixels;
    };

    enum class LoadImageError {
        FileOpenFailed,
        FileReadFailed,
        InputTooLarge,
        DecodeFailed,
    };

    // Decode PNG or JPEG data without creating GPU resources or logging.
    [[nodiscard]] std::expected<LoadImageResult, LoadImageError> decode_image(std::span<const std::uint8_t> encoded);
    [[nodiscard]] std::expected<LoadImageResult, LoadImageError> load_image(const std::filesystem::path& path);
} // namespace orion
