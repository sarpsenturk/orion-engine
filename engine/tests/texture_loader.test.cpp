#include "orion/loaders/texture.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <fstream>

namespace orion
{
    namespace
    {
        // A 2x2 RGBA PNG: red/half-transparent green, transparent blue/white.
        constexpr std::uint8_t png[] = {
            137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82,
            0, 0, 0, 2, 0, 0, 0, 2, 8, 6, 0, 0, 0, 114, 182, 13, 36,
            0, 0, 0, 20, 73, 68, 65, 84, 120, 156, 99, 248, 207, 192, 240,
            31, 8, 27, 24, 192, 52, 16, 0, 0, 63, 215, 8, 121, 143, 19,
            138, 138, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130,
        };

        TEST(TextureLoaderTest, DecodesRgbaWithoutFlippingOrPremultiplying)
        {
            const auto image = decode_image(png);
            ASSERT_TRUE(image.has_value());
            EXPECT_EQ(image->width, 2u);
            EXPECT_EQ(image->height, 2u);
            EXPECT_EQ(image->pixels, (std::vector<std::uint8_t>{
                255, 0, 0, 255, 0, 255, 0, 128,
                0, 0, 255, 0, 255, 255, 255, 255,
            }));
        }

        TEST(TextureLoaderTest, RejectsEmptyAndInvalidData)
        {
            const auto empty = decode_image({});
            ASSERT_FALSE(empty.has_value());
            EXPECT_EQ(empty.error(), LoadImageError::DecodeFailed);
            const std::uint8_t invalid[] = {1, 2, 3, 4};
            const auto malformed = decode_image(invalid);
            ASSERT_FALSE(malformed.has_value());
            EXPECT_EQ(malformed.error(), LoadImageError::DecodeFailed);
            const auto truncated = decode_image(std::span(png).first(32));
            ASSERT_FALSE(truncated.has_value());
            EXPECT_EQ(truncated.error(), LoadImageError::DecodeFailed);
        }

        class TextureLoaderFileTest : public testing::Test
        {
        protected:
            std::filesystem::path directory;

            void SetUp() override
            {
                directory = std::filesystem::temp_directory_path() /
                    ("orion-image-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
                ASSERT_TRUE(std::filesystem::create_directory(directory));
            }

            void TearDown() override
            {
                std::error_code error;
                std::filesystem::remove_all(directory, error);
            }
        };

        TEST_F(TextureLoaderFileTest, LoadsFileWithSamePixelsAsMemory)
        {
            const auto path = directory / "image.png";
            {
                auto file = std::ofstream(path, std::ios::binary);
                file.write(reinterpret_cast<const char*>(png), sizeof(png));
                ASSERT_TRUE(file.good());
            }
            const auto image = load_image(path);
            const auto decoded = decode_image(png);
            ASSERT_TRUE(image.has_value());
            ASSERT_TRUE(decoded.has_value());
            EXPECT_EQ(image->width, decoded->width);
            EXPECT_EQ(image->height, decoded->height);
            EXPECT_EQ(image->pixels, decoded->pixels);
        }

        TEST_F(TextureLoaderFileTest, ReportsMissingFileAndEmptyFile)
        {
            const auto path = directory / "missing.png";
            const auto missing = load_image(path);
            ASSERT_FALSE(missing.has_value());
            EXPECT_EQ(missing.error(), LoadImageError::FileOpenFailed);
            { auto file = std::ofstream(path, std::ios::binary); }
            const auto empty = load_image(path);
            ASSERT_FALSE(empty.has_value());
            EXPECT_EQ(empty.error(), LoadImageError::DecodeFailed);
        }
    } // namespace
} // namespace orion
