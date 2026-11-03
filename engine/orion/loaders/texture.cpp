#include "texture.hpp"

#include "orion/utils/scope_guard.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_NO_STDIO
#include "stb_image.h"

#include <fstream>
#include <limits>

namespace orion
{
    std::expected<LoadImageResult, LoadImageError> decode_image(std::span<const std::uint8_t> encoded)
    {
        if (encoded.empty()) { return std::unexpected(LoadImageError::DecodeFailed); }
        if (encoded.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            return std::unexpected(LoadImageError::InputTooLarge);
        }

        int width = 0;
        int height = 0;
        auto* pixels = stbi_load_from_memory(encoded.data(), static_cast<int>(encoded.size()), &width, &height, nullptr, STBI_rgb_alpha);
        if (!pixels) { return std::unexpected(LoadImageError::DecodeFailed); }
        const auto pixels_guard = ScopeExit{[pixels] { stbi_image_free(pixels); }};

        const auto size = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4u;
        return LoadImageResult{
            .width = static_cast<std::uint32_t>(width),
            .height = static_cast<std::uint32_t>(height),
            .pixels = std::vector<std::uint8_t>(pixels, pixels + size),
        };
    }

    std::expected<LoadImageResult, LoadImageError> load_image(const std::filesystem::path& path)
    {
        auto file = std::ifstream(path, std::ios::binary | std::ios::ate);
        if (!file) { return std::unexpected(LoadImageError::FileOpenFailed); }

        const auto size = file.tellg();
        if (size < 0) { return std::unexpected(LoadImageError::FileReadFailed); }
        if (size > std::numeric_limits<int>::max()) { return std::unexpected(LoadImageError::InputTooLarge); }

        auto encoded = std::vector<std::uint8_t>(static_cast<std::size_t>(size));
        file.seekg(0);
        if (!file.read(reinterpret_cast<char*>(encoded.data()), static_cast<std::streamsize>(encoded.size()))) {
            return std::unexpected(LoadImageError::FileReadFailed);
        }
        return decode_image(encoded);
    }
} // namespace orion
